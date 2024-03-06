#include "property_converter.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

PropertyConverter::PropertyConverter()
{
    // initiate openDAQ logger
    logger = LoggerComponent("PropertyConverterLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

ListPtr<IBaseObject> PropertyConverter::convertJsonArrayToOpendaqList(const Json::Value& jsonArray)
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
                std::string message = "Null type element detected in the Json array!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
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
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        case Json::ValueType::objectValue:
            {
                // TODO: Need to figure out whether object or dict properties are supported in openDAQ under a list 
                std::string message = "ObjectProperty nested under a list is not supported!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        default:
            {
                std::string message = "Unsupported array element type detected: " + arrayElementType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
    }

    return opendaqList;
}

DictPtr<IString, IBaseObject> PropertyConverter::convertJsonDictToOpendaqDict(const Json::Value& jsonDict)
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
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
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
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        case Json::ValueType::objectValue:
            {
                // TODO: Need to figure out whether object or dict properties are supported in openDAQ under a dict
                std::string message = "ObjectProperty nested under dictionaries is not supported!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        default:
            {
                std::string message = "Unsupported dictionary item type detected: " + dictItemType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }

    }    
    
    return opendaqDict;
}

PropertyObjectPtr PropertyConverter::convertJsonObjectToOpendaqObject(const Json::Value& jsonObject, const std::string& pathPrefix)
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
                        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
                    }
                    break;
                default:
                    {
                        std::string message = "Unsupported item type in an ObjectProperty detected!\n";
                        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
                    }
                    break;
            }
        }
    }

    return propertyObject;
}




Json::Value PropertyConverter::convertOpendaqListToJsonArray(const ListPtr<IBaseObject>& opendaqList, const CoreType& listItemType)
{
    Json::Value jsonArray;

    // Return empty Json object if the list is empty
    if(opendaqList.getCount() == 0) 
        return jsonArray;

    switch(listItemType)
    {
        case CoreType::ctBool:
            jsonArray = fillJsonArray_BasicType<bool>(opendaqList);
            break;
        case CoreType::ctInt:
            jsonArray = fillJsonArray_BasicType<int64_t>(opendaqList);
            break;
        case CoreType::ctFloat:
            jsonArray = fillJsonArray_BasicType<double>(opendaqList);
            break;
        case CoreType::ctString:
            jsonArray = fillJsonArray_BasicType<std::string>(opendaqList);
            break;
        case CoreType::ctRatio:
            jsonArray = fillJsonArray_Ratio(opendaqList);
            break;
        case CoreType::ctComplexNumber:
            jsonArray = fillJsonArray_ComplexNumber(opendaqList);
            break;
        default:
            {
                std::string message = "Unsupported list item type: " + listItemType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
    }

    return jsonArray;
}

Json::Value PropertyConverter::convertOpendaqDictToJsonDict(const DictPtr<IString, IBaseObject>& opendaqDict, const CoreType& dictItemType)
{
    Json::Value jsonDict;

    // Return empty Json object if the dict is empty
    if(opendaqDict.getCount() == 0) 
        return jsonDict;

    switch(dictItemType)
    {
        case CoreType::ctBool:
            jsonDict = fillJsonDict_BasicType<bool>(opendaqDict);
            break;
        case CoreType::ctInt:
            jsonDict = fillJsonDict_BasicType<int64_t>(opendaqDict);
            break;
        case CoreType::ctFloat:
            jsonDict = fillJsonDict_BasicType<double>(opendaqDict);
            break;
        case CoreType::ctString:
            jsonDict = fillJsonDict_BasicType<std::string>(opendaqDict);
            break;
        case CoreType::ctRatio:
            {
                std::string message = "RatioProperty nested under dictionaries is not supported!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        case CoreType::ctComplexNumber:
            {
                std::string message = "Complex numbers nested under dictionaries is not supported!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
        default:
            {
                std::string message = "Unsupported dictionary item type: " + dictItemType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
    }

    return jsonDict;
}

template <typename ListItemType>
Json::Value PropertyConverter::fillJsonArray_BasicType(const ListPtr<IBaseObject>& opendaqList)
{
    Json::Value jsonArray;

    for(const ListItemType& item : opendaqList) {
        jsonArray.append(item);
    }

    return jsonArray;
}

Json::Value PropertyConverter::fillJsonArray_Ratio(const ListPtr<IBaseObject>& opendaqList)
{
    Json::Value jsonArray;

    for(const RatioPtr& ratio : opendaqList) {
        Json::Value ratioJson;
        int64_t numerator = ratio.getNumerator();
        int64_t denominator = ratio.getDenominator();
        ratioJson["Numerator"] = numerator;
        ratioJson["Denominator"] = denominator;
        jsonArray.append(ratioJson);
    }

    return jsonArray;
}

Json::Value PropertyConverter::fillJsonArray_ComplexNumber(const ListPtr<IBaseObject>& opendaqList)
{
    Json::Value jsonArray;

    for(const ComplexNumberPtr& complexNumber : opendaqList) {
        Json::Value complexNumberJson;
        double real = complexNumber.getReal();
        double imag = complexNumber.getImaginary();
        complexNumberJson["Real"] = real;
        complexNumberJson["Imaginary"] = imag;
        jsonArray.append(complexNumberJson);
    }

    return jsonArray;
}

template <typename DictItemType>
Json::Value PropertyConverter::fillJsonDict_BasicType(const DictPtr<IString, IBaseObject>& opendaqDict)
{
    Json::Value jsonDict;

    ListPtr<std::string> keyList = opendaqDict.getKeyList(); // Dictionaries with only std::string key types can be represented in Json format!
    ListPtr<DictItemType> itemList = opendaqDict.getValueList();

    for(size_t i = 0; i < opendaqDict.getCount(); i++) {
        std::string key = keyList[i];
        DictItemType value = itemList[i];
        jsonDict[key] = value;
    }

    return jsonDict;
}

void PropertyConverter::convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index)
{
    Json::ValueType jsonValueType = args[index].type();
    switch(jsonValueType)
    {
        case Json::ValueType::nullValue:
            {
                std::string message = "Null argument type detected!\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
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
        {
            std::string message = "Unsupported argument detected: " + jsonValueType + '\n';
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
        }
    }
}

END_NAMESPACE_JET_MODULE