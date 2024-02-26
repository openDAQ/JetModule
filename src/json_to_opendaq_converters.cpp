#include <iostream>
#include "json_to_opendaq_converters.h"

ListPtr<IBaseObject> convertJsonArrayToOpendaqList(const Json::Value& jsonArray)
{
    ListPtr<IBaseObject> opendaqList;

    uint64_t arraySize = jsonArray.size();
    // Return empty openDAQ list if the array is empty
    if(arraySize == 0) 
        return opendaqList;
    Json::ValueType arrayElementType =  jsonArray[0].type();

    switch(arrayElementType)
    {
        case Json::ValueType::nullValue:
            {
                std::string message = "Null type element detected in the array!\n";
                std::cerr << message;
            }
            break;
        case Json::ValueType::booleanValue:
            opendaqList = List<bool>();
            for(int i = 0; i < arraySize; i++) {
                opendaqList.pushBack(jsonArray[i].asBool());
            }
            break;
        case Json::ValueType::intValue:
            opendaqList = List<int64_t>();
            for(int i = 0; i < arraySize; i++) {
                opendaqList.pushBack(jsonArray[i].asInt64());
            }
            break;
        case Json::ValueType::uintValue:
            opendaqList = List<int64_t>(); // There is no unsigned integer CoreType in SDK, so we use integers
            for(int i = 0; i < arraySize; i++) {
                opendaqList.pushBack(jsonArray[i].asUInt64());
            }
            break;
        case Json::ValueType::realValue:
            opendaqList = List<double>();
            for(int i = 0; i < arraySize; i++) {
                opendaqList.pushBack(jsonArray[i].asDouble());
            }
            break;
        case Json::ValueType::stringValue:
            opendaqList = List<std::string>();
            for(int i = 0; i < arraySize; i++) {
                opendaqList.pushBack(jsonArray[i].asString());
            }
            break;
        case Json::ValueType::arrayValue:
            {
                // TODO: Need to figure out whether nested list properties are supported in openDAQ
                std::string message = "Nested list properties are not supported!\n";
                std::cerr << message;
            }
            break;
        case Json::ValueType::objectValue:
            {
                // TODO: Need to figure out whether object or dict properties are supported in openDAQ under a list 
                std::string message = "ObjectProperty nested under a list is not supported!\n";
                std::cerr << message;
            }
            break;
        default:
            {
                std::string message = "Unsupported array element type detected: " + arrayElementType + '\n';
                std::cerr << message;
            }
    }

    return opendaqList;
}

DictPtr<IString, IBaseObject> convertJsonDictToOpendaqDict(const Json::Value& jsonDict)
{
    DictPtr<IString, IBaseObject> opendaqDict;

    uint64_t dictSize = jsonDict.size();
    // Return an empty openDAQ dict if the object is empty
    if(dictSize == 0) 
        return opendaqDict;

    // Getting the first element of the dictionary to determine type of the values afterwards 
    Json::Value::const_iterator iterator = jsonDict.begin();
    Json::Value firstValue = *iterator;

    Json::ValueType dictItemType = firstValue.type();
    switch(dictItemType)
    {
        case Json::ValueType::nullValue:
            {
                std::string message = "Null type element detected in the dictionary!\n";
                std::cerr << message;
            }
            break;
        case Json::ValueType::booleanValue:
            opendaqDict = Dict<std::string, bool>();
            for (Json::Value::const_iterator itr = jsonDict.begin(); itr != jsonDict.end(); ++itr) {
                std::string key = itr.key().asString();
                Json::Value value = *itr;
                opendaqDict.set(key, value.asBool());
            }
            break;
        case Json::ValueType::intValue:
            opendaqDict = Dict<std::string, int64_t>();
            for (Json::Value::const_iterator itr = jsonDict.begin(); itr != jsonDict.end(); ++itr) {
                std::string key = itr.key().asString();
                Json::Value value = *itr;
                opendaqDict.set(key, value.asInt64());
            }
            break;
        case Json::ValueType::uintValue:
            opendaqDict = Dict<std::string, int64_t>();
            for (Json::Value::const_iterator itr = jsonDict.begin(); itr != jsonDict.end(); ++itr) {
                std::string key = itr.key().asString();
                Json::Value value = *itr;
                opendaqDict.set(key, value.asUInt64());
            }
            break;
        case Json::ValueType::realValue:
            opendaqDict = Dict<std::string, double>();
            for (Json::Value::const_iterator itr = jsonDict.begin(); itr != jsonDict.end(); ++itr) {
                std::string key = itr.key().asString();
                Json::Value value = *itr;
                opendaqDict.set(key, value.asDouble());
            }
            break;
        case Json::ValueType::stringValue:
            opendaqDict = Dict<std::string, std::string>();
            for (Json::Value::const_iterator itr = jsonDict.begin(); itr != jsonDict.end(); ++itr) {
                std::string key = itr.key().asString();
                Json::Value value = *itr;
                opendaqDict.set(key, value.asString());
            }
            break;
        case Json::ValueType::arrayValue:
            {
                // TODO: Need to figure out whether list properties are supported in openDAQ under a dict
                std::string message = "List properties nested under dictionaries is not supported!\n";
                std::cerr << message; 
            }
            break;
        case Json::ValueType::objectValue:
            {
                // TODO: Need to figure out whether object or dict properties are supported in openDAQ under a dict
                std::string message = "ObjectProperty nested under dictionaries is not supported!\n";
                std::cerr << message; 
            }
            break;
        default:
            {
                std::string message = "Unsupported dictionary item type detected: " + dictItemType + '\n';
                std::cerr << message;
            }

    }    
    
    return opendaqDict;
}

PropertyObjectPtr convertJsonObjectToOpendaqObject(const Json::Value& jsonObject, const std::string& pathPrefix)
{
    auto propertyObject = PropertyObject();

    for (const auto& key : jsonObject.getMemberNames()) {
        const Json::Value& value = jsonObject[key];

        // Construct the path for the current element
        std::string currentPath = pathPrefix.empty() ? key : pathPrefix + "." + key;

        if (value.isObject()) {
            // Recursively convert nested objects
            PropertyObjectPtr nestedObject = convertJsonObjectToOpendaqObject(value, currentPath);
            propertyObject.addProperty(ObjectProperty(key, nestedObject));
        } 
        else {
            // Process the leaf element
            switch(value.type()) {
                case Json::ValueType::booleanValue:
                   propertyObject.addProperty(BoolProperty(key, value.asBool()));
                    break;
                case Json::ValueType::intValue:
                    propertyObject.addProperty(IntProperty(key, value.asInt64()));
                    break;
                case Json::ValueType::uintValue:
                    propertyObject.addProperty(IntProperty(key, value.asInt64()));
                    break;
                case Json::ValueType::realValue:
                    propertyObject.addProperty(FloatProperty(key, value.asDouble()));
                    break;
                case Json::ValueType::stringValue:
                    propertyObject.addProperty(StringProperty(key, value.asString()));
                    break;
                case Json::ValueType::arrayValue:
                    {
                        std::string message = "Lists nested under ObjectProperty is not supported!\n";
                        std::cerr << message; 
                    }
                    break;
                default:
                    {
                        std::string message = "Unsupported item type in an ObjectProperty detected!\n";
                        std::cerr << message; 
                    }
                    break;
            }
        }
    }

    return propertyObject;
}