#include <iostream>
#include "json_daq_conversion.h"

void convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index)
{
    Json::ValueType jsonValueType = args[index].type();
    switch(jsonValueType)
    {
        case Json::ValueType::nullValue:
            std::cout << "Null argument type detected" << std::endl;
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
            std::cout << "Unsupported argument detected: " << jsonValueType << std::endl;
    }
}

ListPtr<BaseObjectPtr> convertJsonToDaqArray(const ComponentPtr& propertyHolder, const std::string& propertyName, const Json::Value& value)
{
    auto array = value.get(propertyName, "");
    uint64_t arraySize = array.size();
    Json::ValueType arrayElementType =  array[0].type();

    ListPtr<BaseObjectPtr> daqArray;
    switch(arrayElementType)
    {
        case Json::ValueType::nullValue:
            std::cout << "Null type element detected in the array" << std::endl;
            break;
        case Json::ValueType::intValue:
            daqArray = List<int>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asInt());
            }
            break;
        case Json::ValueType::uintValue:
            daqArray = List<uint>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asUInt());
            }
            break;
        case Json::ValueType::realValue:
            daqArray = List<double>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asDouble());
            }
            break;
        case Json::ValueType::stringValue:
            daqArray = List<std::string>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asString());
            }
            break;
        case Json::ValueType::booleanValue:
            daqArray = List<bool>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asBool());
            }
            break;
        default:
            std::cout << "Unsupported array element type detected: " << arrayElementType << std::endl;
    }

    return daqArray;
}

std::string removeSubstring(const std::string& originalString, const std::string& substring) {
    std::string modifiedString = originalString;
    size_t pos = modifiedString.find(substring);

    if (pos != std::string::npos) { // Check if the substring is found
        modifiedString.erase(pos, substring.length());
    }

    return modifiedString;
}
