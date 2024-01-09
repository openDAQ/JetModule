#include <gtest/gtest.h>
#include "jet_server_test.h"

// Checks whether all of the required Jet states are present
TEST_F(JetServerTest, CheckStatePresence)
{
    myJet->publishJetStates();
    
    // Read Jet states
    readJetStates();

    // Check whether all of the Jet states are present
    ASSERT_EQ(jetStateExists(rootDeviceJetPath), true);
    parseFolder(rootDevice, jetStatePath);
}

// Ensures functionality of Boolean property
TEST_F(JetServerTest, TestBoolProperty)
{
    // Add bool property to the device
    std::string propertyName = "TestBool";
    rootDevice.addProperty(BoolProperty(propertyName, true));

    myJet->publishJetStates();

    // Check whether values are equal initially
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            bool valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asBool();
            bool valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            ASSERT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, false);
    readJetStates(); // We have to update publishedJsonValue as we have changed a property value
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            bool valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asBool();
            bool valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value root;
    root[propertyName] = true;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDeviceJetPath, jsonValue.c_str());
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            bool valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asBool();
            bool valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}
}

// Ensures functionality of Integer property
TEST_F(JetServerTest, TestIntProperty)
{
    // Add bool property to the device
    std::string propertyName = "TestInt";
    rootDevice.addProperty(IntProperty(propertyName, 69420));

    myJet->publishJetStates();

    // Check whether values are equal initially
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            int64_t valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asInt64();
            int64_t valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            ASSERT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242);
    readJetStates(); // We have to update publishedJsonValue as we have changed a property value
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            int64_t valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asInt64();
            int64_t valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value root;
    root[propertyName] = 999666;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDeviceJetPath, jsonValue.c_str());
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            int64_t valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asInt64();
            int64_t valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}
}