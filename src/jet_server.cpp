#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    jetPeer = new hbk::jet::PeerAsync(jet_eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

void JetServer::addJetState(const std::string& path)
{
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(path, jsonValue, hbk::jet::responseCallback_t(), cb);
    jsonValue.clear();
}

void JetServer::updateJetState(const PropertyObjectPtr& propertyObject)
{
    ComponentPtr component = propertyObject.asPtr<IComponent>().getObject();
    createJsonProperties(component);   
    appendMetadataToJsonValue(component);

    std::string path = jetStatePath + component.getGlobalId();
    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::publishJetStates()
{
    createComponentJetState(rootDevice);
    auto devices = rootDevice.getDevices();
    createComponentListJetStates(devices);
    auto channels = rootDevice.getChannels();
    createComponentListJetStates(channels);
    auto functionBlocks = rootDevice.getFunctionBlocks();
    createComponentListJetStates(functionBlocks);
    auto customComponents = rootDevice.getCustomComponents();
    createComponentListJetStates(customComponents);
    auto signals = rootDevice.getSignals();
    createComponentListJetStates(signals);
}

void JetServer::createComponentJetState(const ComponentPtr& component)
{
    createJsonProperties(component);   
    appendMetadataToJsonValue(component);
    std::string path = jetStatePath + component.getGlobalId();
    addJetState(path);
}   

void JetServer::createComponentListJetStates(const ListPtr<ComponentPtr>& componentList)
{
    for(auto component : componentList) {
        createComponentJetState(component);
    }
}

void JetServer::createJsonProperty(const ComponentPtr& component, const PropertyPtr& property)
{
    bool isSelectionProperty = determineSelectionProperty(property);
    std::string propertyName = property.getName();
    if(isSelectionProperty) {
        // TODO
        // Every selection property has associated ctInt associated coretype, so to display actual values, std::string is used.
        // This could be changed.
        std::string propertyValue = component.getPropertySelectionValue(toStdString(property.getName().toString()));
        appendPropertyToJsonValue<std::string>(component, propertyName, propertyValue);
    }
    else {
        bool propertyValueBool;
        int64_t propertyValueInt;
        _Float64 propertyValueFloat;
        StringPtr propertyValueString;
        
        CoreType propertyType = property.getValueType();
        switch(propertyType) {
            case CoreType::ctBool:
                propertyValueBool = component.getPropertyValue(property.getName());
                appendPropertyToJsonValue<bool>(component, propertyName, propertyValueBool);
                break;
            case CoreType::ctInt:
                propertyValueInt = component.getPropertyValue(property.getName());
                appendPropertyToJsonValue<int64_t>(component, propertyName, propertyValueInt);
                break;
            case CoreType::ctFloat:
                propertyValueFloat = component.getPropertyValue(property.getName());
                appendPropertyToJsonValue<_Float64>(component, propertyName, propertyValueFloat);
                break;
            case CoreType::ctString:
                propertyValueString = component.getPropertyValue(property.getName());
                appendPropertyToJsonValue<std::string>(component, propertyName, toStdString(propertyValueString));
                break;
            default:
                std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
                std::cout << "\"std::string\" will be used to store property value." << std::endl;
                auto propertyValue = component.getPropertyValue(property.getName());
                appendPropertyToJsonValue<std::string>(component, propertyName, propertyValue);
                break;
        }
    }
}

void JetServer::createJsonProperties(const ComponentPtr& component)
{
    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty(component, property);
        createCallbackForProperty(property);
    }
}

template <typename ValueType>
void JetServer::appendPropertyToJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value)
{
    std::string componentName = toStdString(component.getName());
    jsonValue[componentName][propertyName] = value;
}

void JetServer::appendMetadataToJsonValue(const ComponentPtr& component)
{
    std::string componentName = toStdString(component.getName());
    std::string globalId = toStdString(component.getGlobalId());
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();
    jsonValue[componentName][typeString] = objectType;
    jsonValue[componentName][globalIdString] = globalId;
}

bool JetServer::determineSelectionProperty(const PropertyPtr& property)
{
    return property.getSelectionValues().assigned() ? true : false;
}

void JetServer::createCallbackForProperty(const PropertyPtr& property)
{
    property.getOnPropertyValueWrite() += [&](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) {
        updateJetState(obj);
    };
}

END_NAMESPACE_JET_MODULE