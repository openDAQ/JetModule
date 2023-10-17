#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(DevicePtr device)
{
    this->device = device;
    jetPeer = new daq::jet::PeerAsync(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(jetStatePath, jsonValue, daq::jet::responseCallback_t(), cb);
}

void JetServer::publishJetState()
{
    getDeviceProperties();
    getChannelProperties();

    jetPeer->notifyState(jetStatePath, jsonValue);
}

void JetServer::getDeviceProperties()
{
    deviceName = toStdString(device.getName());
    createJsonProperties(device);
}

void JetServer::getChannelProperties()
{
    auto channels = device.getChannels();
    for(auto channel : channels) {
        channelName = toStdString(channel.getName());
        createJsonProperties(channel);
    }
}

void JetServer::createJsonProperties(PropertyObjectPtr propertyObject)
{
    ConstCharPtr propertyObjectType = propertyObject.asPtr<ISerializable>().getSerializeId();
    auto properties = propertyObject.getAllProperties();
    for(auto property : properties) {
        bool isSelectionProperty = determineSelectionProperty(property);
        std::string propertyName = property.getName();
        if(isSelectionProperty) {
            std::string propertyValue = propertyObject.getPropertySelectionValue(toStdString(property.getName().toString()));
            appendJsonValue<std::string>(propertyObjectType, propertyName, propertyValue);
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
                    appendJsonValue<bool>(propertyObjectType, propertyName, propertyValueBool);
                    break;
                case CoreType::ctInt:
                    propertyValueInt = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<int64_t>(propertyObjectType, propertyName, propertyValueInt);
                    break;
                case CoreType::ctFloat:
                    propertyValueFloat = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<_Float64>(propertyObjectType, propertyName, propertyValueFloat);
                    break;
                case CoreType::ctString:
                    propertyValueString = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<std::string>(propertyObjectType, propertyName, toStdString(propertyValueString));
                    break;
                default:
                    std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
                    std::cout << "\"std::string\" will be used to store property value." << std::endl;
                    auto propertyValue = propertyObject.getPropertyValue(property.getName());
                    appendJsonValue<std::string>(propertyObjectType, propertyName, propertyValue);
                    break;
            }
        }
    }
}

template <typename ValueType>
void JetServer::appendJsonValue(ConstCharPtr propertyObjectType, std::string propertyName, ValueType value)
{
    if(strcmp(propertyObjectType, "Device") == 0) {
        jsonValue[deviceName][propertyName] = value;
    }
    else if(strcmp(propertyObjectType, "Channel") == 0) {
        jsonValue[deviceName][channelName][propertyName] = value;
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