#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    propertyCallbacksCreated = false;
    jetPeer = new hbk::jet::PeerAsync(jet_eventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

JetServer::~JetServer()
{
    delete(jetPeer);
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
    ComponentPtr component = propertyObject.asPtr<IComponent>();
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

    propertyCallbacksCreated = true;
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

template <typename PropertyHolderObject>
void JetServer::createJsonProperty(const ComponentPtr& propertyPublisher, const PropertyPtr& property, const PropertyHolderObject& propertyHolderObject)
{
    bool isSelectionProperty = determineSelectionProperty(property);
    std::string propertyName = property.getName();
    if(isSelectionProperty) {
        // TODO
        // Every selection property has associated ctInt associated coretype, so to display actual values, std::string is used.
        // This could be changed.
        std::string propertyValue = propertyHolderObject.getPropertySelectionValue(toStdString(property.getName().toString()));
        appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, propertyValue);
    }
    else {
        bool propertyValueBool;
        int64_t propertyValueInt;
        double propertyValueFloat;
        StringPtr propertyValueString;
        
        CoreType propertyType = property.getValueType();
        switch(propertyType) {
            case CoreType::ctBool:
                propertyValueBool = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<bool>(propertyPublisher, propertyName, propertyValueBool);
                break;
            case CoreType::ctInt:
                propertyValueInt = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<int64_t>(propertyPublisher, propertyName, propertyValueInt);
                break;
            case CoreType::ctFloat:
                propertyValueFloat = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<double>(propertyPublisher, propertyName, propertyValueFloat);
                break;
            case CoreType::ctString:
                propertyValueString = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, toStdString(propertyValueString));
                break;
            default:
                std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
                std::cout << "\"std::string\" will be used to store property value." << std::endl;
                auto propertyValue = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, propertyValue);
                break;
        }
    }
}

void JetServer::createJsonProperties(const ComponentPtr& component)
{
    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty<ComponentPtr>(component, property, component);
        if(!propertyCallbacksCreated)
            createCallbackForProperty(property);
    }

    // Checking whether the component is a device. If it's a device we have to get deviceInfo properties manually
    if(strcmp(component.asPtr<ISerializable>().getSerializeId(), "Device") == 0) {
        auto deviceInfo = component.asPtr<IDevice>().getInfo();
        auto deviceInfoProperties = deviceInfo.getAllProperties();
        for(auto property : deviceInfoProperties) {
            createJsonProperty<DeviceInfoPtr>(component, property, deviceInfo);
            if(!propertyCallbacksCreated)
                createCallbackForProperty(property);
        }
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