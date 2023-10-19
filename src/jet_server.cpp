#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(DevicePtr device)
{
    this->rootDevice = device;
    jetPeer = new daq::jet::PeerAsync(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(jetStatePath, jsonValue, daq::jet::responseCallback_t(), cb);
}

void JetServer::publishJetState()
{
    parseRootDeviceProperties();
    parseDeviceProperties();
    parseChannelProperties();
    parseFunctionBlockProperties();
    parseCustomComponentProperties();

    jetPeer->notifyState(jetStatePath, jsonValue);
}

void JetServer::parseRootDeviceProperties()
{
    rootDeviceName = toStdString(rootDevice.getName());
    createJsonProperties(rootDevice, rootDeviceType);
}

void JetServer::parseDeviceProperties()
{
    auto devices = rootDevice.getDevices();
    for(auto device : devices) {
        deviceName = toStdString(device.getName());
        createJsonProperties(device, deviceType);   
    }
}

void JetServer::parseChannelProperties()
{
    auto channels = rootDevice.getChannels();
    for(auto channel : channels) {
        channelName = toStdString(channel.getName());
        createJsonProperties(channel, channelType);
    }
}

void JetServer::parseFunctionBlockProperties()
{
    auto functionBlocks = rootDevice.getFunctionBlocks();
    for(auto fb : functionBlocks) {
        functionBlockName = toStdString(fb.getName());
        createJsonProperties(fb, functionBlockType);
    }
}

void JetServer::parseCustomComponentProperties()
{
    auto customComponents = rootDevice.getCustomComponents();
    for(auto customComponent : customComponents) {
        customComponentName = toStdString(customComponent.getName());
        createJsonProperties(customComponent, customComponentType);
    }
}

void JetServer::createJsonProperties(PropertyObjectPtr propertyObject, ConstCharPtr objectType)
{
    // ConstCharPtr propertyObjectType = propertyObject.asPtr<ISerializable>().getSerializeId();
    auto properties = propertyObject.getAllProperties();
    for(auto property : properties) {
        bool isSelectionProperty = determineSelectionProperty(property);
        std::string propertyName = property.getName();
        if(isSelectionProperty) {
            std::string propertyValue = propertyObject.getPropertySelectionValue(toStdString(property.getName().toString()));
            appendJsonValue<std::string>(objectType, propertyName, propertyValue);
        }
        else {
            bool propertyValueBool;
            int64_t propertyValueInt;
            _Float64 propertyValueFloat;
            StringPtr propertyValueString;
            
            CoreType propertyType = determinePropertyType(property);
            switch(propertyType) {
                case CoreType::ctBool:
                    propertyValueBool = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<bool>(objectType, propertyName, propertyValueBool);
                    break;
                case CoreType::ctInt:
                    propertyValueInt = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<int64_t>(objectType, propertyName, propertyValueInt);
                    break;
                case CoreType::ctFloat:
                    propertyValueFloat = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<_Float64>(objectType, propertyName, propertyValueFloat);
                    break;
                case CoreType::ctString:
                    propertyValueString = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<std::string>(objectType, propertyName, toStdString(propertyValueString));
                    break;
                default:
                    std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
                    std::cout << "\"std::string\" will be used to store property value." << std::endl;
                    auto propertyValue = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<std::string>(objectType, propertyName, propertyValue);
                    break;
            }
        }
    }
}

template <typename ValueType>
void JetServer::appendJsonValue(ConstCharPtr objectType, std::string propertyName, ValueType value)
{
    if(strcmp(objectType, rootDeviceType) == 0) {
        jsonValue[rootDeviceName][propertyName] = value;
    }
    else if(strcmp(objectType, deviceType) == 0) {
        jsonValue[rootDeviceName][deviceName][propertyName] = value;
    }
    else if(strcmp(objectType, channelType) == 0) {
        jsonValue[rootDeviceName][channelName][propertyName] = value;
    }
    else if(strcmp(objectType, functionBlockType) == 0) {
        jsonValue[rootDeviceName][functionBlockName][propertyName] = value;
    }
    else if(strcmp(objectType, customComponentType) == 0) {
        jsonValue[rootDeviceName][customComponentName][propertyName] = value;
    }
}

bool JetServer::determineSelectionProperty(PropertyPtr property)
{
    return property.getSelectionValues().assigned() ? true : false;
}

CoreType JetServer::determinePropertyType(PropertyPtr property)
{
    return property.getValueType();
}

END_NAMESPACE_JET_MODULE