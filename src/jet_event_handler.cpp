#include "jet_event_handler.h"
#include "jet_module_exceptions.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

JetEventHandler::JetEventHandler() : jetPeerWrapper(JetPeerWrapper::getInstance())
{

}

/**
 * @brief Addresses to a property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue Json object representing new value of the property.
 */
void JetEventHandler::updateProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newPropertyValue)
{
    PropertyPtr property = component.getProperty(propertyName);
    bool isReadOnly = property.getReadOnly();
    if(isReadOnly) {
        std::string message = "Property \"" + propertyName + "\" is read-only. Its value cannot be changed. Skipping.";
        DAQLOG_W(jetModuleLogger, message.c_str());
        return;
    }

    CoreType propertyType = component.getProperty(propertyName).getValueType();

    std::string unsupportedPropertyType = "Update of property with CoreType " + propertyType + std::string(" is currently unsupported. Skipping.");

    switch(propertyType) {
        case CoreType::ctBool:
            updateSimpleProperty<bool>(component, propertyName, newPropertyValue.asBool());
            break;
        case CoreType::ctInt:
            updateSimpleProperty<int64_t>(component, propertyName, newPropertyValue.asInt64());
            break;
        case CoreType::ctFloat:
            updateSimpleProperty<double>(component, propertyName, newPropertyValue.asDouble());
            break;
        case CoreType::ctString:
            updateSimpleProperty<std::string>(component, propertyName, newPropertyValue.asString());
            break;
        case CoreType::ctList:
            updateListProperty(component, propertyName, newPropertyValue);
            break;
        case CoreType::ctDict:
            updateDictProperty(component, propertyName, newPropertyValue);
            break;
        case CoreType::ctRatio:
            DAQLOG_W(jetModuleLogger, unsupportedPropertyType.c_str());
            break;
        case CoreType::ctComplexNumber:
            DAQLOG_W(jetModuleLogger, unsupportedPropertyType.c_str());
            break;
        case CoreType::ctStruct:
            {
                // Struct is an immutable object in openDAQ and it cannot be modified.
                std::string message = "\"" + propertyName + "\" is StructProperty and cannot be modified.";
                DAQLOG_E(jetModuleLogger, message.c_str());
            }
            break;
        case CoreType::ctObject:
            {
                // ObjectProperty is represented as a separate state. It has to be updated with "updateObjectProperty" function call.
                // This function must not be called for updating ObjectProperty!
                std::string message = "\"" + propertyName + "\" is ObjectProperty and has to be represented as a separate state.";
                DAQLOG_E(jetModuleLogger, message.c_str());
            }
            break;
        case CoreType::ctProc:
            {
                std::string message = "\"" + propertyName + "\" is FunctionProperty and cannot be modified.";
                DAQLOG_E(jetModuleLogger, message.c_str());
            }
            break;
        case CoreType::ctFunc:
            {
                std::string message = "\"" + propertyName + "\" is FunctionProperty and cannot be modified.";
                DAQLOG_E(jetModuleLogger, message.c_str());
            }
            break;
        default:
            DAQLOG_W(jetModuleLogger, unsupportedPropertyType.c_str());
            break;
    }
}

/**
 * @brief Addresses to a simple property (BoolProperty, IntProperty, FloatProperty and StringProperty) value change initiated by a Jet peer.
 * 
 * @tparam DataType Type of the property (bool, int64_t, double or std::string).
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue New value of the property received from Jet.
 */
template <typename DataType>
void JetEventHandler::updateSimpleProperty(const ComponentPtr& component, const std::string& propertyName, const DataType& newPropertyValue)
{
    component.setPropertyValue(propertyName, newPropertyValue);
}

/**
 * @brief Addresses to a list property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newJsonArray Json object containing new value of the list property.
 */
void JetEventHandler::updateListProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonArray)
{
    ListPtr<IBaseObject> newOpendaqList = propertyConverter.convertJsonArrayToOpendaqList(newJsonArray);
    component.setPropertyValue(propertyName, newOpendaqList);
}

/**
 * @brief Addresses to a dict property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newJsonDict Json object containing new value of the dict property.
 */
void JetEventHandler::updateDictProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonDict)
{
    DictPtr<IString, IBaseObject> newOpendaqDict = propertyConverter.convertJsonDictToOpendaqDict(newJsonDict);
    component.setPropertyValue(propertyName, newOpendaqDict);
}

/**
 * @brief Handles ObjectProperty (CoreType::ctObject) nested property value updates initiated by a Jet peer.
 * 
 * @param component Component who owns the ObjectProperty.
 * @param newJsonObject New value of the whole ObjectProperty represented as a Json object.
 */
void JetEventHandler::updateObjectProperty(const ComponentPtr& component, const Json::Value& newJsonObject)
{
    // A vector of path&value pairs representing nested properties within ObjectProperty and their corresponding values
    auto pathAndValuePairs = extractObjectPropertyPathsAndValues(newJsonObject);

    // Updating nested property values
    for(const auto& pair : pathAndValuePairs) {
        updateProperty(component, pair.first, pair.second);
    }
}

/**
 * @brief Addresses to "Active" status change initiated by a Jet peer.
 * 
 * @param component Component whose "Active" status is changed.
 * @param newActiveStatus Json object containing new value for "Active" status.
 */
void JetEventHandler::updateActiveStatus(const ComponentPtr& component, const Json::Value& newActiveStatus)
{
    component.setActive(newActiveStatus.asBool());
}

/**
 * @brief Extracts property paths and corresponding value from ObjectProperty presented as Json object. To access nested properties within
 * ObjectProperty, paths to the property must be provided. This function extracts all those paths to easy-up access to nested properties
 * and change their values with more simplicity.
 * 
 * @param objectPropertyJetState Json representation of ObjectProperty (CoreType::ctObject).
 * @return Vector of path & value pairs representing nested properties.
 */
std::vector<std::pair<std::string, Json::Value>> JetEventHandler::extractObjectPropertyPathsAndValues(const Json::Value& objectPropertyJetState)
{
    std::vector<std::pair<std::string, Json::Value>> pathAndValuePairs;
    extractObjectPropertyPathsAndValuesInternal(objectPropertyJetState, "", pathAndValuePairs);
    return pathAndValuePairs;
}

/**
 * @brief Recursive function which extract path & value pairs from ObjectProperty presented in Json format.
 * 
 * @param objectPropertyJetState Json representation of ObjectProperty (CoreType::ctObject).
 * @param path Path to the ObjectProperty/Array/Nested property - depends on the step in a recursive process.
 * @param pathAndValuePairs Vector of path & value pairs representing nested properties. It is filled in this function.
 */
void JetEventHandler::extractObjectPropertyPathsAndValuesInternal(const Json::Value& objectPropertyJetState, const std::string& path, std::vector<std::pair<std::string, Json::Value>>& pathAndValuePairs)
{
    if(objectPropertyJetState.isObject()) {
        for(const auto& key : objectPropertyJetState.getMemberNames()) {
            std::string newPath = path.empty() ? key : path + "." + key;
            extractObjectPropertyPathsAndValuesInternal(objectPropertyJetState[key], newPath, pathAndValuePairs);
        }
    }
    else if(objectPropertyJetState.isArray()) {
        for(Json::ArrayIndex i = 0; i < objectPropertyJetState.size(); ++i) {
            extractObjectPropertyPathsAndValuesInternal(objectPropertyJetState[i], path, pathAndValuePairs);
        }
    }
    else {
        pathAndValuePairs.push_back({path, objectPropertyJetState});
    }
}

END_NAMESPACE_JET_MODULE