#pragma once
#include <opendaq/opendaq.h>
#include <jet_server.h>
#include <jet/peerasync.hpp>
#include <jet/peer.hpp>
#include <json/value.h>
#include <json/writer.h>


using namespace daq;
using namespace daq::modules::jet_module;
using namespace hbk::jet;

// Json value which is published by JetModule
Json::Value publishedJsonValue;
static hbk::sys::EventLoop eventloop;
static void responseCb(const Json::Value& value)
{
    // value contains the data as an array of objects
    publishedJsonValue = value[hbk::jsonrpc::RESULT];
	// // value contains the data as an array of objects
	// const Json::Value& resultArrary = value[hbk::jsonrpc::RESULT];
	// for (const Json::Value &item: resultArrary) {
	// 	std::cout << "path " << item[hbk::jet::PATH] << std::endl;
	// 	std::cout << "value " << item[hbk::jet::VALUE] << std::endl;
	// }
	// stop whole program afterwards
	eventloop.stop();
}

void readJetStates()
{
    std::string address("127.0.0.1");
    unsigned int port = hbk::jet::JETD_TCP_PORT;
    hbk::jet::matcher_t match;
    hbk::jet::PeerAsync peer(eventloop, address, port);

    peer.getAsync(match, &responseCb);
    eventloop.execute();
}

void modifyJetState(const char* valueType, const std::string& path, const char* newValue)
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
            std::cerr << "invalid value for boolean expecting 'true'', or 'false'" << std::endl;
            // return EXIT_FAILURE;
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
            std::cerr << "error while parsing json!" << std::endl;
            // return EXIT_FAILURE;
        }
    }
}

bool jetStateExists(std::string path)
{
    for (const Json::Value &item: publishedJsonValue) {
        if(item[hbk::jet::PATH] == path)
            return true;
	}
    return false;
}

void parseFolder(const FolderPtr& parentFolder, const std::string& jetStatePath)
{
    auto items = parentFolder.getItems();
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto component = item.asPtrOrNull<IComponent>();

       if (channel.assigned())
        {
            std::string globalId = jetStatePath + channel.getGlobalId();
            EXPECT_EQ(jetStateExists(globalId), true);
        }
        else if (folder.assigned()) // It is important to test for folder last as a channel also is a folder!
        {
            parseFolder(folder, jetStatePath); // Folders are recursively parsed until non-folder items are identified in them
        }
        else if (component.assigned())  // It is important to test for component after folder!
        {
            std::string globalId = jetStatePath + component.getGlobalId();
            EXPECT_EQ(jetStateExists(globalId), true);
        }
    }
}