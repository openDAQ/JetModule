#include "jet_peer_wrapper.h"
#include <opendaq/logger_component_factory.h>
#include <jet/peer.hpp>

BEGIN_NAMESPACE_JET_MODULE

static hbk::sys::EventLoop jetStateReadEventloop;

JetPeerWrapper::JetPeerWrapper()
{
    jetEventloopRunning = false; // TODO: This probably has to be removed

    startJetEventloopThread();
    jetPeer = new hbk::jet::PeerAsync(jetEventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

JetPeerWrapper::~JetPeerWrapper()
{
    stopJetEventloop();
    delete(jetPeer);
}

/**
 * @brief Publishes a Json value as a Jet state to the specified path.
 * 
 * @param path 
 * @param jetState Path which the Jet state will have.
 * @param callback Callback function which will be called when the Jet state is modified.
 */
void JetPeerWrapper::publishJetState(const std::string& path, const Json::Value& jetState, JetStateCallback callback)
{
    jetPeer->addStateAsync(path, jetState, hbk::jet::responseCallback_t(), callback);
}

/**
 * @brief Publishes a Jet method to the specified path.
 * 
 * @param path Path which the Jet method will have.
 * @param callback Callback function which will be called when the Jet method is executed.
 */
void JetPeerWrapper::publishJetMethod(const std::string& path, JetMethodCallback callback)
{
    jetPeer->addMethodAsync(path, hbk::jet::responseCallback_t(), callback);
}

/**
 * @brief Removes a Jet method from the specified path.
 * 
 * @param path Path of the existing Jet method.
 */
void JetPeerWrapper::removeJetMethod(const std::string& path)
{
    jetPeer->removeMethodAsync(path);
}

/**
 * @brief Reads a Jet state with specified path into a Json object.
 * 
 * @param path Path of the Jet state which is read.
 * @return Json::Value object containing a Json representation of the Jet state.
 */
Json::Value JetPeerWrapper::readJetState(const std::string& path)
{
    std::string address("127.0.0.1"); // localhost
    unsigned int port = hbk::jet::JETD_TCP_PORT;

    // We want to get a Jet state with provided path only
    hbk::jet::matcher_t match;
    match.equals = path;

    hbk::jet::PeerAsync jetStateReaderPeer(jetStateReadEventloop, address, port);

    // Create a promise and future
    std::promise<Json::Value> promise;
    std::future<Json::Value> future = promise.get_future();

    // Calls the callback function with the promise
    jetStateReaderPeer.getAsync(match, [&promise](const Json::Value& value) {
        readJetStateCb(promise, value);
    });

    jetStateReadEventloop.execute();

    // Wait for the future to get the value
    Json::Value jetState = future.get();

    // Making sure that size of the array of Json objects is exactly 1
    if(jetState.size() == 0) {
        std::string message = "Could not read Jet state with path: " + path + "\n";
        DAQLOG_E(jetModuleLogger, message.c_str());
    }
    else if(jetState.size() != 1) {
        std::string message = "There are multiple Jet states with path: " + path + "\n";
        DAQLOG_E(jetModuleLogger, message.c_str());
    }

    // We get the first Json object in the array and get its value afterwards (Json object comes with path&value pair, we only need value)
    jetState = jetState[0][hbk::jet::VALUE];

    return jetState;
}

/**
 * @brief Reads all Jet states into a Json object.
 * 
 * @return Json::Value object containing a Json representation of all the Jet states.
 */
Json::Value JetPeerWrapper::readAllJetStates()
{
    std::string address("127.0.0.1");
    unsigned int port = hbk::jet::JETD_TCP_PORT;
    hbk::jet::matcher_t match;
    hbk::jet::PeerAsync peer(jetStateReadEventloop, address, port);

    // Create a promise and future
    std::promise<Json::Value> promise;
    std::future<Json::Value> future = promise.get_future();

    // Calls the callback function with the promise
    peer.getAsync(match, [&promise](const Json::Value& value) {
        readJetStateCb(promise, value);
    });

    jetStateReadEventloop.execute();

    // Wait for the future to get the value
    Json::Value result = future.get();
    return result;
}

/**
 * @brief Overwrites an existing Jet state with provided Json value.
 * 
 * @param path Path of the Jet state.
 * @param newValue Json value which will be overwritten in the Jet state.
 */
void JetPeerWrapper::updateJetState(const std::string& path, const Json::Value newValue)
{
    jetPeer->notifyState(path, newValue);
}

/**
 * @brief Callback function used in Jet state reader functions. It sets assign Json value to std::promise when called.
 * 
 * @param promise Container which is assigned Json value containing Jet state(s).
 * @param value Json value containing Jet state(s).
 */
void JetPeerWrapper::readJetStateCb(std::promise<Json::Value>& promise, const Json::Value& value)
{
    // value contains the data as an array of objects
    Json::Value jetState = value[hbk::jsonrpc::RESULT];
    promise.set_value(jetState);

    // Stop the event loop
    jetStateReadEventloop.stop();
}

/**
 * @brief Modifies Jet state.
 * 
 * @param valueType Type of the value that has to be modified. As all of the Jet states besides the methods are published with Json types,
 * use "json" as an argument.
 * @param path Path of the Jet state that is needed to be modified.
 * @param newValue New value of the Jet state.
 */
void JetPeerWrapper::modifyJetState(const char* valueType, const std::string& path, const char* newValue)
{
    unsigned int port = hbk::jet::JETD_TCP_PORT;
    std::string address("127.0.0.1");
    hbk::jet::Peer peer(address, port);
    // hbk::jet::PeerAsync peer(eventloop, address, port);
    if(strcmp(valueType, "bool") == 0) 
    {
        if(strcmp(newValue, "false") == 0)
        {
            peer.setStateValue(path, false, 2.71828182846);
        } 
        else if (strcmp(newValue, "true") == 0) 
        {
            peer.setStateValue(path, true, 2.71828182846);
        } 
        else 
        {
            std::string message = "Could not modify Jet state with path: " + path + "\n" + 
                "invalid value for boolean expecting 'true'', or 'false'\n";
            DAQLOG_E(jetModuleLogger, message.c_str());
        }
    } 
    else if(strcmp(valueType,"int")==0) 
    {
        int value = atoi(newValue);
        peer.setStateValue(path, value, 2.71828182846);
    } 
    else if(strcmp(valueType, "double")==0) 
    {
        double value = strtod(newValue, nullptr);
        peer.setStateValue(path, value, 2.71828182846);
    } 
    else if(strcmp(valueType,"string")==0) 
    {
        peer.setStateValue(path, newValue, 2.71828182846);
    } 
    else if(strcmp(valueType,"json")==0) 
    {
        Json::Value params;

        Json::CharReaderBuilder rBuilder;
        if(rBuilder.newCharReader()->parse(newValue, newValue+strlen(newValue), &params, nullptr)) 
        {
            peer.setStateValue(path, params, 2.71828182846);
        } 
        else 
        {
            std::string message = "Could not modify Jet state with path: " + path + "\n" + 
                "error while parsing json!\n";
            DAQLOG_E(jetModuleLogger, message.c_str());
        }
    }
}

/**
 * @brief Helper function used in ComponentConverter callback to remove root device ID from other components' global IDs.
 * 
 * @param path Path of Jet state, same as global ID of a component.
 * @return std::string which has root device ID removed.
 */
std::string JetPeerWrapper::removeRootDeviceId(const std::string& path)
{
    std::string relativePath = path;
    // Find the position of the first slash
    size_t firstSlashPos = relativePath.find("/");
    // Find the position of the second slash, starting the search from the character after the first slash
    size_t secondSlashPos = relativePath.find("/", firstSlashPos + 1);

    // Check if both slashes are found
    if (firstSlashPos != std::string::npos && secondSlashPos != std::string::npos) {
        // If both slashes are found, erase the substring between them (including the second slash)
        // The '+1' in the length calculation includes the removal of the second slash
        relativePath.erase(firstSlashPos, secondSlashPos - firstSlashPos + 1);
    } else {
        // If not both slashes are found, clear the entire string
        relativePath.clear();
    }

    return relativePath;
}

/**
 * @brief Removes last string after '/' (slash) character from a Jet path string. It's used to remove ObjectProperty name from its Jet state path.
 * This is needed to retrieve the component which owns the ObjectProperty.
 * 
 * @param path Path of the Jet state.
 * @return Path of the Jet state with ObjectProperty name removed.
 */
std::string JetPeerWrapper::removeObjectPropertyName(const std::string& path)
{
    // Find the last occurrence of '/'
    size_t lastSlashPos = path.rfind('/');

    // If '/' is found, return the substring from the beginning up to the last '/'
    // Otherwise, return the original string
    if (lastSlashPos != std::string::npos) {
        return path.substr(0, lastSlashPos);
    } else {
        return path;
    }
}

void JetPeerWrapper::startJetEventloop()
{
    if(!jetEventloopRunning) {
        jetEventloopRunning = true;
        jetEventloop.execute();
    }
}

void JetPeerWrapper::stopJetEventloop()
{
    if(jetEventloopRunning) {
        jetEventloopRunning = false;
        jetEventloop.stop();
        jetEventloopThread.join();
    }
}

void JetPeerWrapper::startJetEventloopThread()
{
    jetEventloopThread = std::thread{ &JetPeerWrapper::startJetEventloop, this };
}

END_NAMESPACE_JET_MODULE