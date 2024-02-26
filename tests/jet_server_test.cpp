#include <gtest/gtest.h>
#include "jet_server_test.h"
#include "opendaq_to_json_converters.h"
#include "json_to_opendaq_converters.h"

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
    valueInJet = getPropertyValueInJet(propertyName).asBool();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, false);
    valueInJet = getPropertyValueInJet(propertyName).asBool();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    bool newValue = true;
    setPropertyValueInJet(propertyName, newValue);
    valueInJet = getPropertyValueInJetTimeout(propertyName, newValue).asBool();
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
    valueInJet = getPropertyValueInJet(propertyName).asInt64();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242);
    valueInJet = getPropertyValueInJet(propertyName).asInt64();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    int64_t newValue = 999666;
    setPropertyValueInJet(propertyName, newValue);
    valueInJet = getPropertyValueInJetTimeout(propertyName, newValue).asInt64();
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
    valueInJet = getPropertyValueInJet(propertyName).asDouble();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, 424242.12413);
    valueInJet = getPropertyValueInJet(propertyName).asDouble();
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    double newValue = 999666.77713;
    setPropertyValueInJet(propertyName, newValue);
    valueInJet = getPropertyValueInJetTimeout(propertyName, newValue).asDouble();
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
    valueInJet = getPropertyValueInJet(propertyName).asString();
    std::string valueInOpendaq1 = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq1);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, "Richard Dawkins");
    valueInJet = getPropertyValueInJet(propertyName).asString();
    std::string valueInOpendaq2 = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq2);

    // Check whether property value updated from Jet updates value in openDAQ
    std::string newValue = "John Cena";
    setPropertyValueInJet(propertyName, newValue);
    valueInJet = getPropertyValueInJetTimeout(propertyName, newValue).asString();
    std::string valueInOpendaq3 = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq3);
}

// Ensures functionality of List property
TEST_F(JetServerTest, TestListProperty)
{
    ListPtr<IBaseObject> valueInJet;
    ListPtr<IBaseObject> valueInOpendaq;

    // Add list property to the device
    std::string propertyName = "TestList";
    rootDevice.addProperty(ListProperty(propertyName, List<std::string>("Georgia", "is", "beautiful")));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = convertJsonArrayToOpendaqList(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, List<std::string>("Optimus", "Prime"));
    valueInJet = convertJsonArrayToOpendaqList(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    std::vector<std::string> newValue = {"The", "first", "principle", "is", "that", "you", "must", "not", "fool", "yourself", "and", "you", "are", "the", "easiest", "person", "to", "fool."};
    setPropertyListInJet(propertyName, newValue);
    valueInJet = convertJsonArrayToOpendaqList(getPropertyValueInJetTimeout(propertyName, convertVectorToJson(newValue)));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}

// Ensures functionality of Dict property
TEST_F(JetServerTest, TestDictProperty)
{
    DictPtr<IString, IBaseObject> valueInJet;
    DictPtr<IString, IBaseObject> valueInOpendaq;

    // Add list property to the device
    std::string propertyName = "TestDict";
    DictPtr<IString, Int> dict = Dict<IString, int64_t>();
    dict.set("A", 1);
    dict.set("B", 2);
    dict.set("C", 3);
    rootDevice.addProperty(DictProperty(propertyName, dict));

    jetServer->publishJetStates();

    // Check whether values are equal initially
    valueInJet = convertJsonDictToOpendaqDict(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    // dict.clear();
    dict = Dict<IString, int64_t>();
    dict.set("FirstElement", 321321);
    dict.set("SecondElement", 666777);
    rootDevice.setPropertyValue(propertyName, dict);
    valueInJet = convertJsonDictToOpendaqDict(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value dictJson(Json::objectValue);
    dictJson["Element1"] = 2;
    dictJson["Element2"] = 3;
    dictJson["Element3"] = 5;
    setPropertyValueInJet(propertyName, dictJson);
    valueInJet = convertJsonDictToOpendaqDict(getPropertyValueInJetTimeout(propertyName, dictJson));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}
