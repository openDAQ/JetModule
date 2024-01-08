#include <gtest/gtest.h>
#include <opendaq/opendaq.h>
#include <jet_server.h>
#include <jet/peerasync.hpp>
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

// Checks whether all of the required Jet states are present
TEST(TestJetServer, CheckStatePresence)
{
    // Create an openDAQ instance, loading modules at MODULE_PATH
    const daq::InstancePtr instance = daq::Instance(MODULE_PATH);
    // Add a reference device as root device
    instance.setRootDevice("daqref://device0");
    // Start streaming openDAQ OpcUa server
    instance.addServer("openDAQ OpcUa", nullptr);
    // Get a root device from instance
    auto rootDevice = instance.getRootDevice();

    // Publish device structure as Jet states
    JetServer myJet = JetServer(rootDevice);
    myJet.publishJetStates();
    
    // Read Jet states
    readJetStates();

    // Check whether all of the Jet states are present
    std::string jetStatePath = myJet.getJetStatePath();
    std::string rootDeviceGlobalId = jetStatePath + rootDevice.getGlobalId();
    ASSERT_EQ(jetStateExists(rootDeviceGlobalId), true);
    parseFolder(rootDevice, jetStatePath);
}