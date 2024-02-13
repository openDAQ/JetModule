#include <gtest/gtest.h>
#include "jet_server_test.h"

// Checks whether all of the required Jet states are present
TEST_F(JetServerTest, CheckStatePresence)
{
    // Publish device structure as Jet states
    jetServer->publishJetStates();
    
    // Read Jet states
    Json::Value jetStates = readJetStates();
    // Get Jet state paths
    std::vector<std::string> jetStatePaths = getJetStatePaths(jetStates);
    // Get component IDs of openDAQ objects
    std::vector<std::string> globalIds = getComponentIDs(rootDevice);

    // Check whether there same amount of states in Jet as there are high level objects in openDAQ
    ASSERT_EQ(jetStatePaths.size(), globalIds.size());

    // Sort both vectors to ensure elements are in a consistent order
    std::sort(jetStatePaths.begin(), jetStatePaths.end());
    std::sort(globalIds.begin(), globalIds.end());
    // Check whether individual Jet state paths match global IDs of openDAQ objects
    bool statesMatch = std::equal(jetStatePaths.begin(), jetStatePaths.end(), globalIds.begin());
    ASSERT_TRUE(statesMatch);
}

// Ensures functionality of Boolean property
TEST_F(JetServerTest, TestBoolProperty)
{
    bool valueInJet;
    bool valueInOpendaq;

    // Add bool property to the device
    std::string propertyName = "TestBool";
    rootDevice.addProperty(BoolProperty(propertyName, true));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = getPropertyValueInJet(propertyName, false).asBool();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, false);
    valueInJet = getPropertyValueInJet(propertyName, true).asBool();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    setPropertyValueInJet(propertyName, true);
    valueInJet = getPropertyValueInJet(propertyName, false).asBool();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}

// Ensures functionality of Integer property
TEST_F(JetServerTest, TestIntProperty)
{
    int64_t valueInJet;
    int64_t valueInOpendaq;

    // Add integer property to the device
    std::string propertyName = "TestInt";
    rootDevice.addProperty(IntProperty(propertyName, 69420));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = getPropertyValueInJet(propertyName, -1).asInt64();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242);
    valueInJet = getPropertyValueInJet(propertyName, -1).asInt64();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    setPropertyValueInJet(propertyName, 999666);
    valueInJet = getPropertyValueInJet(propertyName, -1).asInt64();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}

// Ensures functionality of Float property
TEST_F(JetServerTest, TestFloatProperty)
{
    double valueInJet;
    double valueInOpendaq;

    // Add float property to the device
    std::string propertyName = "TestFloat";
    rootDevice.addProperty(FloatProperty(propertyName, 69420.69));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = getPropertyValueInJet(propertyName, -1).asDouble();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242.12413);
    valueInJet = getPropertyValueInJet(propertyName, -1).asDouble();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    setPropertyValueInJet(propertyName, 999666.77713);
    valueInJet = getPropertyValueInJet(propertyName, -1).asDouble();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}

// Ensures functionality of String property
TEST_F(JetServerTest, TestStringProperty)
{
    std::string valueInJet;
    // std::string valueInOpendaq;

    // Add string property to the device
    std::string propertyName = "TestString";
    rootDevice.addProperty(StringProperty(propertyName, "Richard Feynman"));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = getPropertyValueInJet(propertyName, "").asString();
    std::string valueInOpendaq1 = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq1);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, "Richard Dawkins");
    valueInJet = getPropertyValueInJet(propertyName, "").asString();
    std::string valueInOpendaq2 = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq2);

    // Check whether property value updated from Jet updates value in openDAQ
    setPropertyValueInJet(propertyName, "John Cena");
    valueInJet = getPropertyValueInJet(propertyName, "").asString();
    std::string valueInOpendaq3 = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq3);
}

// Ensures functionality of List property
TEST_F(JetServerTest, TestListProperty)
{
    ListPtr<BaseObjectPtr> valueInJet;
    ListPtr<BaseObjectPtr> valueInOpendaq;

    // Add list property to the device
    std::string propertyName = "TestList";
    rootDevice.addProperty(ListProperty(propertyName, List<std::string>("Georgia", "is", "beautiful")));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = jetServer->convertJsonArrayToDaqArray(rootDevice, propertyName, getPropertyListInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, List<std::string>("Optimus", "Prime"));
    valueInJet = jetServer->convertJsonArrayToDaqArray(rootDevice, propertyName, getPropertyListInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    setPropertyListInJet(propertyName, {"The", "first", "principle", "is", "that", "you", "must", "not", "fool", "yourself", "and", "you", "are", "the", "easiest", "person", "to", "fool."});
    valueInJet = jetServer->convertJsonArrayToDaqArray(rootDevice, propertyName, getPropertyListInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}
