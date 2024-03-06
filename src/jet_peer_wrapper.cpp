#include "jet_peer_wrapper.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

static hbk::sys::EventLoop jetStateReadEventloop;

JetPeerWrapper::JetPeerWrapper()
{
    // initiate openDAQ logger
    logger = LoggerComponent("JetPeerWrapperLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);

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
 * @param path Path which the Jet state will have.
 * @param jetState Json representation of the Jet state.
 */
void JetPeerWrapper::publishJetState(const std::string& path, const Json::Value& jetState, JetStateCallback callback)
{
    jetPeer->addStateAsync(path, jetState, hbk::jet::responseCallback_t(), callback);
}

void JetPeerWrapper::publishJetMethod(const std::string& path, JetMethodCallback callback)
{
    jetPeer->addMethodAsync(path, hbk::jet::responseCallback_t(), callback);
}

/**
 * @brief Read a Jet state with specified path into a Json object.
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
        std::string message = "Could not read Jet state with path: " + path + "!\n";
        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
    }
    else if(jetState.size() != 1) {
        std::string message = "There are multiple Jet states with path: " + path + "!\n";
        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
    }

    // We get the first Json object in the array and get its value afterwards (Json object comes with path&value pair, we only need value)
    jetState = jetState[0][hbk::jet::VALUE];

    return jetState;
}

void JetPeerWrapper::updateJetState(const std::string& path, const Json::Value newValue)
{
    jetPeer->notifyState(path, newValue);
}

void JetPeerWrapper::readJetStateCb(std::promise<Json::Value>& promise, const Json::Value& value)
{
    // value contains the data as an array of objects
    Json::Value jetState = value[hbk::jsonrpc::RESULT];
    promise.set_value(jetState);

    // Stop the event loop
    jetStateReadEventloop.stop();
}

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