#include <iostream>
#include "jet_server_base.h"

BEGIN_NAMESPACE_JET_MODULE

static hbk::sys::EventLoop jetStateReadEventloop;

JetServerBase::JetServerBase()
{
    // initiate openDAQ logger
    logger = LoggerComponent("JetModuleLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

void JetServerBase::convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index)
{
    Json::ValueType jsonValueType = args[index].type();
    switch(jsonValueType)
    {
        case Json::ValueType::nullValue:
            {
                std::string message = "Null argument type detected!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        case Json::ValueType::intValue:
            daqArg.asPtr<IList>().pushBack(args[index].asInt());
            break;
        case Json::ValueType::uintValue:
            daqArg.asPtr<IList>().pushBack(args[index].asUInt());
            break;
        case Json::ValueType::realValue:
            daqArg.asPtr<IList>().pushBack(args[index].asDouble());
            break;
        case Json::ValueType::stringValue:
            daqArg.asPtr<IList>().pushBack(args[index].asString());
            break;
        case Json::ValueType::booleanValue:
            daqArg.asPtr<IList>().pushBack(args[index].asBool());
            break;
        default:
        {
            std::string message = "Unsupported argument detected: " + jsonValueType + '\n';
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
        }
    }
}

/**
 * @brief Read a Jet state with specified path into a Json object.
 * 
 * @param path Path of the Jet state which is read.
 * @return Json::Value object containing a Json representation of the Jet state.
 */
Json::Value JetServerBase::readJetState(const std::string& path)
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
        //TODO! Need to throw an exception
    }
    else if(jetState.size() != 1) {
        //TODO! Need to throw an exception
    }

    // We get the first Json object in the array and get its value afterwards (Json object comes with path&value pair, we only need value)
    jetState = jetState[0][hbk::jet::VALUE];

    return jetState;
}

void JetServerBase::readJetStateCb(std::promise<Json::Value>& promise, const Json::Value& value)
{
    // value contains the data as an array of objects
    Json::Value jetState = value[hbk::jsonrpc::RESULT];
    promise.set_value(jetState);

    // Stop the event loop
    jetStateReadEventloop.stop();
}

// TODO! This function is not finished
void JetServerBase::modifyJetState(const std::string& path, const std::string& entryName, const Json::Value& entryValue)
{
    Json::Value jetStateBefore = readJetState(path);
    
}

std::string JetServerBase::removeRootDeviceId(const std::string& path)
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

END_NAMESPACE_JET_MODULE