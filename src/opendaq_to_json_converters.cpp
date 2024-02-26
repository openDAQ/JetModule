#include <iostream>
#include "opendaq_to_json_converters.h"

// Creating an unnamed namespace to limit the scope of access to these internal functions to this translation unit
namespace {
    template <typename ListItemType>
    Json::Value fillJsonArray_BasicType(const ListPtr<IBaseObject>& opendaqList);
    Json::Value fillJsonArray_Ratio(const ListPtr<IBaseObject>& opendaqList);
    Json::Value fillJsonArray_ComplexNumber(const ListPtr<IBaseObject>& opendaqList);

    template <typename DictItemType>
    Json::Value fillJsonDict_BasicType(const DictPtr<IString, IBaseObject>& opendaqDict);
}

Json::Value convertOpendaqListToJsonArray(const ListPtr<IBaseObject>& opendaqList, const CoreType& listItemType)
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
                std::cerr << message;
            }
            break;
    }

    return jsonArray;
}

Json::Value convertOpendaqDictToJsonDict(const DictPtr<IString, IBaseObject>& opendaqDict, const CoreType& dictItemType)
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
                std::cerr << message;
            }
            break;
        case CoreType::ctComplexNumber:
            {
                std::string message = "Complex numbers nested under dictionaries is not supported!\n";
                std::cerr << message;
            }
            break;
        default:
            {
                std::string message = "Unsupported dictionary item type: " + dictItemType + '\n';
                std::cerr << message;
            }
            break;
    }

    return jsonDict;
}

namespace {

    template <typename ListItemType>
    Json::Value fillJsonArray_BasicType(const ListPtr<IBaseObject>& opendaqList)
    {
        Json::Value jsonArray;

        for(const ListItemType& item : opendaqList) {
            jsonArray.append(item);
        }

        return jsonArray;
    }

    Json::Value fillJsonArray_Ratio(const ListPtr<IBaseObject>& opendaqList)
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

    Json::Value fillJsonArray_ComplexNumber(const ListPtr<IBaseObject>& opendaqList)
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
    Json::Value fillJsonDict_BasicType(const DictPtr<IString, IBaseObject>& opendaqDict)
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
}