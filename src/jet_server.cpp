#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    jetPeer = new daq::jet::PeerAsync(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

void JetServer::addJetState(const std::string& path)
{
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(path, jsonValue, daq::jet::responseCallback_t(), cb);
    jsonValue.clear();
}

void JetServer::updateJetState(const ComponentPtr& component, const std::string& propertyName)
{
    std::string globalId = component.getGlobalId();
    std::string path = jetStatePath + globalId;

    auto property = component.getProperty(propertyName);
    createJsonProperty(component, property);

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
    std::string globalId = component.getGlobalId();
    std::string path = jetStatePath + globalId;
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
        std::string propertyValue = component.getPropertySelectionValue(toStdString(property.getName().toString()));
        appendJsonValue<std::string>(component, propertyName, propertyValue);
    }
    else {
        bool propertyValueBool;
        int64_t propertyValueInt;
        _Float64 propertyValueFloat;
        StringPtr propertyValueString;
        
        CoreType propertyType = property.getValueType();;
        switch(propertyType) {
            case CoreType::ctBool:
                propertyValueBool = component.getPropertyValue(property.getName());
                appendJsonValue<bool>(component, propertyName, propertyValueBool);
                break;
            case CoreType::ctInt:
                propertyValueInt = component.getPropertyValue(property.getName());
                appendJsonValue<int64_t>(component, propertyName, propertyValueInt);
                break;
            case CoreType::ctFloat:
                propertyValueFloat = component.getPropertyValue(property.getName());
                appendJsonValue<_Float64>(component, propertyName, propertyValueFloat);
                break;
            case CoreType::ctString:
                propertyValueString = component.getPropertyValue(property.getName());
                appendJsonValue<std::string>(component, propertyName, toStdString(propertyValueString));
                break;
            default:
                std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
                std::cout << "\"std::string\" will be used to store property value." << std::endl;
                auto propertyValue = component.getPropertyValue(property.getName());
                appendJsonValue<std::string>(component, propertyName, propertyValue);
                break;
        }
    }
}

void JetServer::createJsonProperties(const ComponentPtr& component)
{
    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty(component, property);
    }
}

template <typename ValueType>
void JetServer::appendJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value)
{
    std::string componentName = toStdString(component.getName());
    std::string globalId = toStdString(component.getGlobalId());
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();

    jsonValue[componentName][propertyName] = value;
    jsonValue[componentName][typeString] = objectType;
    jsonValue[componentName][globalIdString] = globalId;
}

bool JetServer::determineSelectionProperty(const PropertyPtr& property)
{
    return property.getSelectionValues().assigned() ? true : false;
}

END_NAMESPACE_JET_MODULE