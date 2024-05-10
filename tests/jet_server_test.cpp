#include <gtest/gtest.h>
#include "jet_server_test.h"
#include <coreobjects/argument_info_factory.h>
#include <coreobjects/callable_info_factory.h>

// Checks whether all of the required Jet states are present
TEST_F(JetServerTest, CheckStatePresence)
{
    // Get component IDs of openDAQ objects
    std::vector<std::string> globalIds = getComponentIDs();
    // Get Jet state paths
    std::vector<std::string> jetStatePaths = getJetStatePaths();

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

    // Check whether values are equal initially
    valueInJet = propertyConverter.convertJsonArrayToOpendaqList(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    rootDevice.setPropertyValue(propertyName, List<std::string>("Optimus", "Prime"));
    valueInJet = propertyConverter.convertJsonArrayToOpendaqList(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    std::vector<std::string> newValue = {"The", "first", "principle", "is", "that", "you", "must", "not", "fool", "yourself", "and", "you", "are", "the", "easiest", "person", "to", "fool."};
    setPropertyListInJet(propertyName, newValue);
    valueInJet = propertyConverter.convertJsonArrayToOpendaqList(getPropertyValueInJetTimeout(propertyName, convertVectorToJson(newValue)));
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

    // Check whether values are equal initially
    valueInJet = propertyConverter.convertJsonDictToOpendaqDict(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    ASSERT_EQ(valueInJet, valueInOpendaq);
 
    // Check whether property value updated from openDAQ updates value in Jet
    // dict.clear();
    dict = Dict<IString, int64_t>();
    dict.set("FirstElement", 321321);
    dict.set("SecondElement", 666777);
    rootDevice.setPropertyValue(propertyName, dict);
    valueInJet = propertyConverter.convertJsonDictToOpendaqDict(getPropertyValueInJet(propertyName));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);

    // Check whether property value updated from Jet updates value in openDAQ
    Json::Value dictJson(Json::objectValue);
    dictJson["Element1"] = 2;
    dictJson["Element2"] = 3;
    dictJson["Element3"] = 5;
    setPropertyValueInJet(propertyName, dictJson);
    valueInJet = propertyConverter.convertJsonDictToOpendaqDict(getPropertyValueInJetTimeout(propertyName, dictJson));
    valueInOpendaq = rootDevice.getPropertyValue(propertyName);
    EXPECT_EQ(valueInJet, valueInOpendaq);
}

TEST_F(JetServerTest, TestFunctionPropertyProcedure)
{
    hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0, "callingPeer");
    double timeout = 50; // 50ms

    int testVar = 0;


    // Procedure with no argument
    std::string propName = "TestProcNoArg";
    const auto procPropertyNoArg = FunctionProperty(propName, ProcedureInfo());
    rootDevice.addProperty(procPropertyNoArg);
    rootDevice.setPropertyValue(propName, Procedure([&testVar] () { testVar = 10; }));

    std::string path = rootDevicePath + "/" + propName;
    Json::Value result = callingPeer.callMethod(path, Json::Value(), timeout);
    ASSERT_EQ(testVar, 10);

    // Procedure with single argument
    propName = "TestProcSingleArg";
    const auto procPropertySingleArg = FunctionProperty(propName, ProcedureInfo(List<IArgumentInfo>(ArgumentInfo("arg", CoreType::ctInt))));
    rootDevice.addProperty(procPropertySingleArg);
    rootDevice.setPropertyValue(propName, Procedure([&testVar] (int arg) { testVar = arg; }));

    path = rootDevicePath + "/" + propName;
    result = callingPeer.callMethod(path, 20, timeout);
    ASSERT_EQ(testVar, 20);


    // Procedure with single argument provided as a list
    propName = "TestProcSingleArgAsList";
    const auto procPropertySingleArgAsList = FunctionProperty(propName, ProcedureInfo(List<IArgumentInfo>(ArgumentInfo("arg", CoreType::ctInt))));
    rootDevice.addProperty(procPropertySingleArgAsList);
    rootDevice.setPropertyValue(propName, Procedure([&testVar] (int arg) { testVar = arg; }));

    path = rootDevicePath + "/" + propName;
    Json::Value jsonArray;
    jsonArray.append(30);
    result = callingPeer.callMethod(path, jsonArray, timeout);
    ASSERT_EQ(testVar, 30);
    

    // Procedure with multiple arguments of different types
    propName = "TestProcMultipleArg";
    const auto procPropertyMultipleArg = FunctionProperty(propName, ProcedureInfo(List<IArgumentInfo>(ArgumentInfo("arg1", CoreType::ctInt),
                                                                                                    ArgumentInfo("arg2", CoreType::ctFloat),
                                                                                                    ArgumentInfo("arg3", CoreType::ctBool),
                                                                                                    ArgumentInfo("arg4", CoreType::ctString))));
    rootDevice.addProperty(procPropertyMultipleArg);
    rootDevice.setPropertyValue(propName, Procedure([&testVar] (int arg1, double arg2, bool arg3, std::string arg4) { testVar = arg1; }));

    path = rootDevicePath + "/" + propName;
    jsonArray.clear();
    jsonArray.append(40);
    jsonArray.append(420.69);
    jsonArray.append(true);
    jsonArray.append("Georgia");
    result = callingPeer.callMethod(path, jsonArray, timeout);
    ASSERT_EQ(testVar, 40);
}

TEST_F(JetServerTest, TestFunctionPropertyFunction)
{
    hbk::jet::Peer callingPeer(hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0, "callingPeer");
    double timeout = 50; // 50ms


    // Function with no argument
    std::string propName = "TestFuncNoArg";
    const auto procPropertyNoArg = FunctionProperty(propName, FunctionInfo(CoreType::ctInt));
    rootDevice.addProperty(procPropertyNoArg);
    rootDevice.setPropertyValue(propName, Function([] () { return 42; }));

    std::string path = rootDevicePath + "/" + propName;
    Json::Value result = callingPeer.callMethod(path, Json::Value(), timeout);
    ASSERT_EQ(result.asInt(), 42);


    // Function with single argument
    propName = "TestFuncSingleArg";
    const auto procPropertySingleArg = FunctionProperty(propName, FunctionInfo(CoreType::ctInt, List<IArgumentInfo>(ArgumentInfo("arg", CoreType::ctInt))));
    rootDevice.addProperty(procPropertySingleArg);
    rootDevice.setPropertyValue(propName, Function([] (int arg) { return arg; }));

    path = rootDevicePath + "/" + propName;
    result = callingPeer.callMethod(path, 69, timeout);
    ASSERT_EQ(result.asInt(), 69);


    // Function with single argument provided as a list
    propName = "TestFuncSingleArgAsList";
    const auto procPropertySingleArgAsList = FunctionProperty(propName, FunctionInfo(CoreType::ctInt, List<IArgumentInfo>(ArgumentInfo("arg", CoreType::ctInt))));
    rootDevice.addProperty(procPropertySingleArgAsList);
    rootDevice.setPropertyValue(propName, Function([] (int arg) { return arg; }));

    path = rootDevicePath + "/" + propName;
    Json::Value jsonArray;
    jsonArray.append(420);
    result = callingPeer.callMethod(path, jsonArray, timeout);
    ASSERT_EQ(result.asInt(), 420);


    // Function with multiple arguments
    propName = "TestFuncMultipleArg";
    const auto procPropertyMultipleArg = FunctionProperty(propName, FunctionInfo(CoreType::ctFloat, List<IArgumentInfo>(ArgumentInfo("arg1", CoreType::ctFloat),
                                                                                                                    ArgumentInfo("arg2", CoreType::ctFloat),
                                                                                                                    ArgumentInfo("arg3", CoreType::ctFloat))));
    rootDevice.addProperty(procPropertyMultipleArg);
    rootDevice.setPropertyValue(propName, Function([] (double arg1, double arg2, double arg3) { return (arg1+arg2+arg3)/3; }));

    path = rootDevicePath + "/" + propName;
    jsonArray.clear();
    jsonArray.append(10);
    jsonArray.append(20);
    jsonArray.append(30);
    result = callingPeer.callMethod(path, jsonArray, timeout);
    ASSERT_EQ(result.asInt(), 20);
}