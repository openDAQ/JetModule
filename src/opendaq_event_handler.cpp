#include "opendaq_event_handler.h"
#include <json/value.h>
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

OpendaqEventHandler::OpendaqEventHandler() : jetPeerWrapper(JetPeerWrapper::getInstance())
{

}

/**
 * @brief Addresses to a property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    std::string propertyPath = eventParameters.get("Path");

    // Represents the property path taking its owner Component as a starting point
    // This is needed when the property is nested under ObjectProperty (CoreType::ctObject)
    std::string fullPath = propertyPath.empty() ? propertyName : propertyPath + "." + propertyName; 
    
    PropertyPtr property = component.getProperty(fullPath);
    CoreType propertyType = property.getValueType();

    std::string message = "Update of property with CoreType " + propertyType + std::string(" is not supported currently.\n");

    switch(propertyType) {
        case CoreType::ctBool:
            updateSimpleProperty<bool>(component, eventParameters);
            break;
        case CoreType::ctInt:
            updateSimpleProperty<int64_t>(component, eventParameters);
            break;
        case CoreType::ctFloat:
            updateSimpleProperty<double>(component, eventParameters);
            break;
        case CoreType::ctString:
            updateSimpleProperty<std::string>(component, eventParameters);
            break;
        case CoreType::ctList:
            updateListProperty(component, eventParameters);
            break;
        case CoreType::ctDict:
            updateDictProperty(component, eventParameters);
            break;
        case CoreType::ctRatio:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        case CoreType::ctComplexNumber:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        case CoreType::ctStruct:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        case CoreType::ctObject:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        case CoreType::ctProc:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        case CoreType::ctFunc:
            DAQLOG_W(jetModuleLogger, message.c_str());
            break;
        default:
            break;
    }

}

/**
 * @brief Addresses to a simple property (BoolProperty, IntProperty, FloatProperty and StringProperty) value change initiated 
 * by openDAQ client/server.
 * 
 * @tparam DataType Type of the property (bool, int64_t, double or std::string).
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
template <typename DataType>
void OpendaqEventHandler::updateSimpleProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    std::string propertyPath = eventParameters.get("Path");

    DataType propertyValue = eventParameters.get("Value");

    bool isNestedProperty = propertyPath.empty();
    std::string componentId = component.getGlobalId();
    std::vector<std::string> nestedPropertyNames; // These are the names of ObjectProperties under which the property that has been updated is nested

    std::string jetStatePath;
    if(isNestedProperty)
        jetStatePath = componentId;
    else {
        nestedPropertyNames = extractNestedPropertyNames(propertyPath);
        jetStatePath = componentId + "/" + nestedPropertyNames[0]; // ObjectProperty (CoreType::ctObject) is reperesnted as a separate Jet state
    }
    Json::Value jetState = jetPeerWrapper.readJetState(jetStatePath); // Jet state before the change

    if(isNestedProperty)
        jetState[propertyName] = propertyValue;
    else {
        setNestedPropertyValue<DataType>(jetState, nestedPropertyNames, propertyName, propertyValue);
    }

    jetPeerWrapper.updateJetState(jetStatePath, jetState);
}

/**
 * @brief Addresses to a list property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateListProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    std::string propertyPath = eventParameters.get("Path");

    ListPtr<IBaseObject> propertyValue = eventParameters.get("Value");
    CoreType listItemType = component.getProperty(propertyName).getItemType();
    Json::Value propertyValueJson = propertyConverter.convertOpendaqListToJsonArray(propertyValue, listItemType);
    
    bool isNestedProperty = propertyPath.empty();
    std::string componentId = component.getGlobalId();
    std::vector<std::string> nestedPropertyNames; // These are the names of ObjectProperties under which the property that has been updated is nested

    std::string jetStatePath;
    if(isNestedProperty)
        jetStatePath = componentId;
    else {
        nestedPropertyNames = extractNestedPropertyNames(propertyPath);
        jetStatePath = componentId + "/" + nestedPropertyNames[0]; // ObjectProperty (CoreType::ctObject) is reperesnted as a separate Jet state
    }
    Json::Value jetState = jetPeerWrapper.readJetState(jetStatePath); // Jet state before the change

    if(isNestedProperty)
        jetState[propertyName] = propertyValueJson;
    else {
        setNestedPropertyValue<Json::Value>(jetState, nestedPropertyNames, propertyName, propertyValueJson);
    }

    jetPeerWrapper.updateJetState(jetStatePath, jetState);
}

/**
 * @brief Addresses to a dict property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateDictProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    std::string propertyPath = eventParameters.get("Path");

    DictPtr<IString, IBaseObject> propertyValue = eventParameters.get("Value");
    CoreType dictItemType = component.getProperty(propertyName).getItemType();
    Json::Value propertyValueJson = propertyConverter.convertOpendaqDictToJsonDict(propertyValue, dictItemType);
    
    bool isNestedProperty = propertyPath.empty();
    std::string componentId = component.getGlobalId();
    std::vector<std::string> nestedPropertyNames; // These are the names of ObjectProperties under which the property that has been updated is nested

    std::string jetStatePath;
    if(isNestedProperty)
        jetStatePath = componentId;
    else {
        nestedPropertyNames = extractNestedPropertyNames(propertyPath);
        jetStatePath = componentId + "/" + nestedPropertyNames[0]; // ObjectProperty (CoreType::ctObject) is reperesnted as a separate Jet state
    }
    Json::Value jetState = jetPeerWrapper.readJetState(jetStatePath); // Jet state before the change

    if(isNestedProperty)
        jetState[propertyName] = propertyValueJson;
    else {
        setNestedPropertyValue<Json::Value>(jetState, nestedPropertyNames, propertyName, propertyValueJson);
    }

    jetPeerWrapper.updateJetState(jetStatePath, jetState);
}

/**
 * @brief Addresses to a "Active" status change initiated by openDAQ client/server.
 * 
 * @param component Component whose "Active" status is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateActiveStatus(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    bool newActiveStatus = eventParameters.get("Active");

    jetState["Active"] = newActiveStatus;
    jetPeerWrapper.updateJetState(path, jetState);
}

/**
 * @brief Addresses a property addition to an openDAQ component. It adds the property to Jet state representing the component.
 * 
 * @param component Component to which a property has been added.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::addProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    // Property name in eventParameters is in "Property {<property_name>}" format, so we have to extract the string between curly braces
    std::string propertyName = extractPropertyName(eventParameters.get("Property"));
    if(propertyName == "") {
        std::string message = "Property has been added to component \"" + component.getName() + "\" but could not extract property's name!\n";
        DAQLOG_E(jetModuleLogger, message.c_str());
        return;
    }

    PropertyPtr property = component.getProperty(propertyName);
    propertyManager.determinePropertyType<ComponentPtr>(component, property, jetState);
    jetPeerWrapper.updateJetState(path, jetState);
}

/**
 * @brief OpenDAQ event, which describes property addition to an openDAQ component, has property's name in the format of 
 * "Property {<property_name>}". So, the string between curly braces has to be extracted. This function does that.
 * 
 * @param str Original string from openDAQ event.
 * @return std::string corresponding to property name.
 */
std::string OpendaqEventHandler::extractPropertyName(const std::string& str)
{
    
    size_t startPos = str.find("{");
    size_t endPos = str.find("}");

    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        // Add 1 to startPos to start from the character after '{'
        // Calculate the length of the content by subtracting the positions, minus one to exclude '}'
        return str.substr(startPos + 1, endPos - startPos - 1);
    }
    
    // Return an empty string to indicate that the format was not as expected
    return "";
}

/**
 * @brief Returns vector of names under which the property is nested. Nested property is under of ObjectProperty(ies) (CoreType::ctObject).
 * This function returns names of the ObjectProperty(ies).
 * 
 * @param objectPropertyPath Path of the property which is nested under ObjectProperty(ies) without including the name of the property. 
 * Path is represented by ObjectProperty names separated with commas.
 * @return std::vector<std::string> Vector of ObjectProperty names under which the property is nested.
 */
std::vector<std::string> OpendaqEventHandler::extractNestedPropertyNames(const std::string& objectPropertyPath)
{
    std::istringstream iss(objectPropertyPath);
    std::string name;
    std::vector<std::string> nestedPropertyNames;

    while(std::getline(iss, name, '.')) {
        nestedPropertyNames.emplace_back(name);
    }

    return nestedPropertyNames;
}

/**
 * @brief Sets the property value which is nested under ObjectProperty(ies) (CoreType::ctObject).
 * 
 * @tparam PropertyType Type of the property which corresponds to types from Json::ValueType.
 * @param jetState Jet state under which the property is located.
 * @param nestedPropertyNames Names of the ObjectProperty(ies) under which the property is nested.
 * @param propertyName Name of the Property.
 * @param propertyValue Value of the property.
 */
template <typename PropertyType>
void OpendaqEventHandler::setNestedPropertyValue(Json::Value& jetState, const std::vector<std::string>& nestedPropertyNames, const std::string& propertyName, const PropertyType& propertyValue)
{
    Json::Value* currentJsonVal = &jetState;
    for(size_t i = 0; i < nestedPropertyNames.size(); ++i) {
        if(i == nestedPropertyNames.size() - 1) { // Last element sets the value
            (*currentJsonVal)[nestedPropertyNames[i]][propertyName] = propertyValue;
        } 
        else { // Intermediate elements navigate/create the structure
            currentJsonVal = &((*currentJsonVal)[nestedPropertyNames[i]]);
        }
    }
}

END_NAMESPACE_JET_MODULE