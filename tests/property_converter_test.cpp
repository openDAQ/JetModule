#include <gtest/gtest.h>
#include <opendaq_to_json_converters.h>
#include "json_to_opendaq_converters.h"

using namespace daq;


TEST(OpendaqToJsonConverters, TestListConversion)
{
    ListPtr<IBaseObject> opendaqList = List<int64_t>(33, 69, 42, 420, 37);
    CoreType listItemType = CoreType::ctInt;

    Json::Value jsonArray = convertOpendaqListToJsonArray(opendaqList, listItemType);

    // Ensure that arrays contain same number of elements
    ASSERT_EQ(jsonArray.size(), opendaqList.getCount());

    // NOTE: sorting of the arrays could be needed before the assertion
    for(int i = 0; i < jsonArray.size(); i++) {
        int64_t valueInJsonArray = jsonArray[i].asInt64();
        int64_t valueInOpendaqList = opendaqList[i];
        EXPECT_EQ(valueInJsonArray, valueInOpendaqList);
    }
}

TEST(OpendaqToJsonConverters, TestDictConversion)
{
    DictPtr<IString, IBaseObject> opendaqDict = Dict<std::string, int64_t>();
    opendaqDict.set("number2", 33);
    opendaqDict.set("number2", 69);
    opendaqDict.set("number3", 42);
    opendaqDict.set("number4", 420);
    opendaqDict.set("number5", 37);
    CoreType dictItemType = CoreType::ctInt;

    Json::Value jsonDict = convertOpendaqDictToJsonDict(opendaqDict, dictItemType);

    ListPtr<std::string> keyList = opendaqDict.getKeyList();

    // Ensure that dictionaries contain same number of key-value pairs
    ASSERT_EQ(jsonDict.size(), opendaqDict.getCount());

    for(std::string key : keyList) {
        ASSERT_TRUE(jsonDict.isMember(key)); // Ensure that Json dict contains the same keys as openDAQ dict

        std::string valueInJsonDict = jsonDict.get(key, "").asString();
        std::string valueInOpendaqDict = opendaqDict.get(key);
        EXPECT_EQ(valueInJsonDict, valueInOpendaqDict);
    }
}

TEST(JsonToOpendaqConverters, TestListConversion)
{
    Json::Value jsonArray(Json::arrayValue);
    jsonArray.append("Khachapuri");
    jsonArray.append("Khinkali");
    jsonArray.append("Mtsvadi");
    jsonArray.append("Kharcho");
    jsonArray.append("Churchkhela");

    ListPtr<IBaseObject> opendaqList = convertJsonArrayToOpendaqList(jsonArray);

    // Ensure that arrays contain same number of elements
    ASSERT_EQ(jsonArray.size(), opendaqList.getCount());

    // NOTE: sorting of the arrays could be needed before the assertion
    for(int i = 0; i < jsonArray.size(); i++) {
        std::string valueInJsonArray = jsonArray[i].asString();
        std::string valueInOpendaqList = opendaqList[i];
        EXPECT_EQ(valueInJsonArray, valueInOpendaqList);
    }
}

TEST(JsonToOpendaqConverters, TestDictConversion)
{
    Json::Value jsonDict;
    jsonDict["animal1"] = "Tortoise";
    jsonDict["animal2"] = "Penguin";
    jsonDict["animal3"] = "Alligator";

    DictPtr<IString, IBaseObject> opendaqDict = convertJsonDictToOpendaqDict(jsonDict);

    ListPtr<std::string> keyList = opendaqDict.getKeyList();

    // Ensure that dictionaries contain same number of key-value pairs
    ASSERT_EQ(jsonDict.size(), opendaqDict.getCount());

    for(std::string key : keyList) {
        ASSERT_TRUE(jsonDict.isMember(key)); // Ensure that Json dict contains the same keys as openDAQ dict

        std::string valueInJsonDict = jsonDict.get(key, "").asString();
        std::string valueInOpendaqDict = opendaqDict.get(key);
        EXPECT_EQ(valueInJsonDict, valueInOpendaqDict);
    }

}

TEST(JsonToOpendaqConverters, TestObjectConversion)
{
    Json::Value grandparent;
    Json::Value parent;
    Json::Value child;

    child["foo"] = 3.14159;
    parent["Child"] = child;
    grandparent["Parent"] = parent;

    PropertyObjectPtr propertyObject = convertJsonObjectToOpendaqObject(grandparent, "");

    // Creating the same openDAQ property object manually. It can't be used in GTest for a direct comparison
    // auto childOpendaq = PropertyObject();
    // childOpendaq.addProperty(FloatProperty("foo", 3.14159));
    // auto parentOpendaq = PropertyObject();
    // parentOpendaq.addProperty(ObjectProperty("Child", childOpendaq));
    // auto grandparentOpendaq = PropertyObject();
    // grandparentOpendaq.addProperty(ObjectProperty("Parent", parentOpendaq));

    ASSERT_EQ(propertyObject.getPropertyValue("Parent.Child.foo"), 3.14159);
}