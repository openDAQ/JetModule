#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(DevicePtr device)
{
    this->rootDevice = device;
    jetPeer = new daq::jet::PeerAsync(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

void JetServer::addJetState(std::string &path)
{
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(path, jsonValue, daq::jet::responseCallback_t(), cb);
    jsonValue.clear();
}

void JetServer::updateJetState(ComponentPtr component, std::string &propertyName)
{
    std::string globalId = component.getGlobalId();
    std::string path = jetStatePath + globalId;

    auto property = component.getProperty(propertyName);
    createJsonProperty(component, property);

    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::publishJetState()
{
    parseRootDeviceProperties();
    parseDeviceProperties();
    parseChannelProperties();
    parseFunctionBlockProperties();
    parseCustomComponentProperties();
    parseSignalProperties();
}

void JetServer::parseRootDeviceProperties()
{
    rootDeviceName = toStdString(rootDevice.getName());
    createJsonProperties(rootDevice);
    
    std::string globalId = rootDevice.getGlobalId();
    std::string path = jetStatePath + globalId;
    addJetState(path);
}

void JetServer::parseDeviceProperties()
{
    auto devices = rootDevice.getDevices();
    for(auto device : devices) {
        deviceName = toStdString(device.getName());
        createJsonProperties(device);   

        std::string globalId = device.getGlobalId();
        std::string path = jetStatePath + globalId;
        addJetState(path);
    }
}

void JetServer::parseChannelProperties()
{
    auto channels = rootDevice.getChannels();
    for(auto channel : channels) {
        channelName = toStdString(channel.getName());
        createJsonProperties(channel);

        std::string globalId = channel.getGlobalId();
        std::string path = jetStatePath + globalId;
        addJetState(path);
    }
}

void JetServer::parseFunctionBlockProperties()
{
    auto functionBlocks = rootDevice.getFunctionBlocks();
    for(auto fb : functionBlocks) {
        functionBlockName = toStdString(fb.getName());
        createJsonProperties(fb);

        std::string globalId = fb.getGlobalId();
        std::string path = jetStatePath + globalId;
        addJetState(path);
    }
}

void JetServer::parseCustomComponentProperties()
{
    auto customComponents = rootDevice.getCustomComponents();
    for(auto customComponent : customComponents) {
        customComponentName = toStdString(customComponent.getName());
        createJsonProperties(customComponent);

        std::string globalId = customComponent.getGlobalId();
        std::string path = jetStatePath + globalId;
        addJetState(path);
    }
}

void JetServer::parseSignalProperties()
{
    auto signals = rootDevice.getSignals();
    for(auto signal : signals) {
        signalName = toStdString(signal.getName());
        createJsonProperties(signal);

        std::string globalId = signal.getGlobalId();
        std::string path = jetStatePath + globalId;
        addJetState(path);
    }
}

void JetServer::createJsonProperty(ComponentPtr component, PropertyPtr property)
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

void JetServer::createJsonProperties(ComponentPtr component)
{
    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty(component, property);
    }
}

template <typename ValueType>
void JetServer::appendJsonValue(ComponentPtr component, std::string propertyName, ValueType value)
{
    std::string componentName = toStdString(component.getName());
    std::string globalId = toStdString(component.getGlobalId());
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();

    jsonValue[componentName][propertyName] = value;
    jsonValue[componentName][typeString] = objectType;
    jsonValue[componentName][globalIdString] = globalId;
}

bool JetServer::determineSelectionProperty(PropertyPtr property)
{
    return property.getSelectionValues().assigned() ? true : false;
}

END_NAMESPACE_JET_MODULE