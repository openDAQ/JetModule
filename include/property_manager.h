/*
 * Copyright 2022-2023 Blueberry d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "common.h"
#include <json/value.h>
#include <opendaq/device_impl.h>
#include "property_converter.h"
#include "jet_peer_wrapper.h"

using namespace daq;

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Container for functions which append different type of properties to a Json value. The Json value will later be published
 * as a Jet state.
 * 
 */
class PropertyManager
{
public:
    explicit PropertyManager();

    // Helper function which determines type of an openDAQ property
    template <typename PropertyHolder>
    void determinePropertyType(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

    // Append properties to Json value
    template<typename PropertyHolderType, typename DataType>
    void appendSimpleProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendListProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendObjectProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendRatioProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendComplexNumber(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendStructProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

    void createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property);

private:
    PropertyConverter propertyConverter;
    JetPeerWrapper& jetPeerWrapper;
    LoggerComponentPtr logger;
};


//! Template functions definitions have to be in the header file so that derived classes are able to use them.
/* 
 * Unlike regular functions and methods, template definitions (including template member functions of a class) must be visible 
 * to a translation unit that uses them. This is because the compiler needs to instantiate the template with the specific type 
 * arguments used in the code, which happens at compile time.
*/

/**
 * @brief Determines type of an openDAQ property in order to correctly represent it in Json format.
 * 
 * @tparam PropertyHolder Type of the object which owns the property.
 * @param propertyHolder Object which owns the property.
 * @param property The property whose type is determined.
 * @param parentJsonValue Json object object to which the property is appended.
 */
template <typename PropertyHolder>
void PropertyManager::determinePropertyType(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    CoreType propertyType = property.getValueType();

    switch(propertyType) {
        case CoreType::ctBool:
            appendSimpleProperty<PropertyHolder, bool>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctInt:
            appendSimpleProperty<PropertyHolder, int64_t>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctFloat:
            appendSimpleProperty<PropertyHolder, double>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctString:
            appendSimpleProperty<PropertyHolder, std::string>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctList:
            appendListProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctDict:
            appendDictProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctRatio:
            appendRatioProperty<PropertyHolder>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctComplexNumber:
            // TODO: Needs verification that it works. Have to create a list of complex numbers
            appendComplexNumber<PropertyHolder>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctStruct:
            appendStructProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctObject:
            {
                PropertyObjectPtr propertyObject = propertyHolder.getPropertyValue(propertyName);
                std::string propertyObjectName = property.getName();

                auto properties = propertyObject.getAllProperties();
                for(auto property : properties)
                {
                    determinePropertyType<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
                }
            }
            break;
        case CoreType::ctProc:
            createJetMethod(propertyHolder, property);
            break;
        case CoreType::ctFunc:
            createJetMethod(propertyHolder, property);
            break;
        default:
            {
                std::string message = "Unsupported value type of Property: " + propertyName + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
                message = "\"std::string\" will be used to store property value.\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                std::string propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
    }
}

/**
 * @brief Appends simple propertiy types such as BoolProperty, IntProperty, FloatProperty and StringProperty to Json object,
 * in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @tparam DataType Type of the property - bool, in64_t, double or std::string.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType, typename DataType>
void PropertyManager::appendSimpleProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue) {
    DataType propertyValue = propertyHolder.getPropertyValue(propertyName);
    parentJsonValue[propertyName] = propertyValue;
}

/**
 * @brief Appends ListProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendListProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    ListPtr<IBaseObject> opendaqList = propertyHolder.getPropertyValue(propertyName);
    CoreType listItemType = property.getItemType();

    Json::Value jsonArray = propertyConverter.convertOpendaqListToJsonArray(opendaqList, listItemType);
    parentJsonValue[propertyName] = jsonArray;
}

/**
 * @brief Appends DictProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    DictPtr<IString, IBaseObject> opendaqDict = propertyHolder.getPropertyValue(propertyName); //! Non-string key types cannot be represented in Json::Value!
    CoreType itemCoreType = property.getItemType();

    Json::Value jsonDict = propertyConverter.convertOpendaqDictToJsonDict(opendaqDict, itemCoreType);
    parentJsonValue[propertyName] = jsonDict;
}

/**
 * @brief Appends RatioProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendRatioProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    RatioPtr ratioProperty = propertyHolder.getPropertyValue(propertyName);
    int64_t numerator = ratioProperty.getNumerator();
    int64_t denominator = ratioProperty.getDenominator();
    parentJsonValue[propertyName]["Numerator"] = numerator;
    parentJsonValue[propertyName]["Denominator"] = denominator;
}

// TODO! This function is not tested
/**
 * @brief Appends ComplexNumber object to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendComplexNumber(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    ComplexNumberPtr complexNumber = propertyHolder.getPropertyValue(propertyName);
    double real = complexNumber.getReal();
    double imag = complexNumber.getImaginary();
    parentJsonValue[propertyName]["Real"] = real;
    parentJsonValue[propertyName]["Imaginary"] = imag;
}

// TODO! This function needs to be finished
/**
 * @brief Appends StructProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendStructProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    StructPtr propertyStruct = propertyHolder.getPropertyValue(propertyName);
    ListPtr<IString> fieldNames = propertyStruct.getFieldNames();
    ListPtr<IBaseObject> fieldValues = propertyStruct.getFieldValues();

    for(size_t i = 0; i < fieldNames.getCount(); i++)
    {
        CoreType structfieldType = fieldValues[i].getCoreType();
        switch(structfieldType)
        {
            case CoreType::ctBool:
                {
                    bool fieldValue = fieldValues[i];
                    parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                }
                break;
            case CoreType::ctInt:
                {
                    int64_t fieldValue = fieldValues[i];
                    parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                }
                break;
            case CoreType::ctFloat:
                {
                    double fieldValue = fieldValues[i];
                    parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                }
                break;
            case CoreType::ctString:
                {
                    std::string fieldValue = fieldValues[i];
                    parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                }
                break;
            case CoreType::ctList:
                {
                    ListPtr<IBaseObject> fieldValue = fieldValues[i];
                    CoreType listItemType = fieldValue.asPtr<IProperty>().getItemType();
                    switch(listItemType)
                    {
                        case CoreType::ctBool:
                            {
                                ListPtr<bool> propertyList = fieldValues[i];

                                for(const bool& item : propertyList) {
                                    parentJsonValue[propertyName][toStdString(fieldNames[i])].append(item);
                                }
                            }
                            break;
                        case CoreType::ctInt:
                            break;
                        case CoreType::ctFloat:
                            break;
                        case CoreType::ctString:
                            // appendListPropertyToJsonValue<std::string>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                            break;
                        default:
                            {
                                // std::string message = "Unsupported list item type: " + listItemType + '\n';
                                // jetPeerWrapper.logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
                            }
                        break;
                    }
                }
            case CoreType::ctDict:
                break;
            case CoreType::ctRatio:
                break;
            case CoreType::ctComplexNumber:
                break;
            case CoreType::ctStruct:
                break;
            case CoreType::ctEnumeration:
                break;
            default:
                {
                    // std::string message = "Unsupported struct field type: " + structfieldType + '\n';
                    // jetPeerWrapper.logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                }
        }
    }

    std::string structValue = propertyHolder.getPropertyValue(propertyName);
    parentJsonValue[propertyName] = structValue;
}

/**
 * @brief Appends ObjectProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void PropertyManager::appendObjectProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    PropertyObjectPtr propertyObject = propertyHolder.getPropertyValue(propertyName);
    std::string propertyObjectName = property.getName();

    auto properties = propertyObject.getAllProperties();
    for(auto property : properties)
    {
        determinePropertyType<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
    }
}

END_NAMESPACE_JET_MODULE