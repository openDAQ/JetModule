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

// Ensures functionality of String property
TEST_F(JetServerTest, TestStringProperty)
{
    // Add bool property to the device
    std::string propertyName = "TestString";
    rootDevice.addProperty(StringProperty(propertyName, "Richard Feynman"));

    myJet->publishJetStates();

    // Check whether values are equal initially
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            std::string valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asString();
            std::string valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            ASSERT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, "Richard Dawkins");
    readJetStates(); // We have to update publishedJsonValue as we have changed a property value
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            std::string valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asString();
            std::string valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value root;
    root[propertyName] = "John Cena";
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDeviceJetPath, jsonValue.c_str());
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            std::string valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asString();
            std::string valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}
}

// Ensures functionality of Float property
TEST_F(JetServerTest, TestFloatProperty)
{
    // Add bool property to the device
    std::string propertyName = "TestFloat";
    rootDevice.addProperty(FloatProperty(propertyName, 69420.69));

    myJet->publishJetStates();

    // Check whether values are equal initially
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            double valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asDouble(); // in openDAQ ctFloat CoreType is actually double
            double valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            ASSERT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242.12413);
    readJetStates(); // We have to update publishedJsonValue as we have changed a property value
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            double valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asDouble();
            double valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value root;
    root[propertyName] = 999666.77713;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDeviceJetPath, jsonValue.c_str());
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            double valueInJet = item[hbk::jet::VALUE].get(propertyName, "").asDouble();
            double valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}
}

// Ensures functionality of List property
TEST_F(JetServerTest, TestListProperty)
{
    // Add bool property to the device
    std::string propertyName = "TestList";
    rootDevice.addProperty(ListProperty(propertyName, List<std::string>("Georgia", "is", "beautiful")));

    myJet->publishJetStates();

    // Check whether values are equal initially
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            ListPtr<BaseObjectPtr> valueInJet = myJet->convertJsonArrayToDaqArray(rootDevice, propertyName, item[hbk::jet::VALUE]);
            ListPtr<BaseObjectPtr> valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            ASSERT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, List<std::string>("Optimus", "Prime"));
    readJetStates(); // We have to update publishedJsonValue as we have changed a property value
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            ListPtr<BaseObjectPtr> valueInJet = myJet->convertJsonArrayToDaqArray(rootDevice, propertyName, item[hbk::jet::VALUE]);
            ListPtr<BaseObjectPtr> valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value root;
    Json::Value array(Json::arrayValue);
    std::vector<std::string> values = {"The", "first", "principle", "is", "that", "you", "must", "not", "fool", "yourself", "and", "you", "are", "the", "easiest", "person", "to", "fool."};
    for (auto val : values) {
        array.append(val);
    }
    root[propertyName] = array;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDeviceJetPath, jsonValue.c_str());
    readJetStates();
    for (const Json::Value& item : publishedJsonValue) {
        if(item[hbk::jet::PATH] == rootDeviceJetPath)
        {
            ListPtr<BaseObjectPtr> valueInJet = myJet->convertJsonArrayToDaqArray(rootDevice, propertyName, item[hbk::jet::VALUE]);
            ListPtr<BaseObjectPtr> valueInOpendaq = rootDevice.getPropertyValue(propertyName);
            EXPECT_EQ(valueInJet, valueInOpendaq);
        }
	}
}