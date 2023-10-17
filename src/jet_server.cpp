#include "jet_server.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(DevicePtr device)
{
    this->device = device;
    deviceName = toStdString(device.getName());
    jetPeer = new daq::jet::PeerAsync(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(jetStatePath, jsonValue, daq::jet::responseCallback_t(), cb);
}

void JetServer::deviceLoop()
{
    getDeviceProperties();
    getChannelProperties();

    jetPeer->notifyState(jetStatePath, jsonValue);

    // for(int i = 0; i < 20; i++) {
    //     std::string myString = std::to_string(i);
    //     jsonValue[deviceName][myString] = "skibidipipihahahahazzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
    //     jetPeer->notifyState(jetStatePath, jsonValue);
    // }
}

void JetServer::getDeviceProperties()
{
    auto properties = device.getAllProperties();
    for(auto property : properties) {
        bool isSelectionProperty = determineSelectionProperty(property);
        std::string propertyName = property.getName();
        if(isSelectionProperty) {
            std::string propertyValue = device.getPropertySelectionValue(toStdString(property.getName().toString()));
            jsonValue[deviceName][propertyName] = propertyValue;
            // std::cout << "Property: " << property.getName();
            // std::cout << " " << isSelectionProperty << std::endl;
        }
        else {
            CoreType propertyType = determinePropertyType(property);
            std::string propertyValue = device.getPropertyValue(property.getName());
            jsonValue[deviceName][propertyName] = propertyValue;
            // std::cout << "Property: " << property.getName();
            // std::cout << " " << isSelectionProperty << "   " << propertyType << std::endl;
        }
        // jetPeer->notifyState(jetStatePath, jsonValue);
    }
    // jetPeer->notifyState(jetStatePath, jsonValue);
}

void JetServer::getChannelProperties()
{
    auto channels = device.getChannels();
    for(auto channel : channels) {
        auto channelProperties = channel.getAllProperties();
        std::string channelName = channel.getName();
        // std::cout << std::endl << "Channel: " << channel.getName() << std::endl;
        for(auto property : channelProperties) {
            bool isSelectionProperty = determineSelectionProperty(property);
            std::string propertyName = property.getName();
            if(isSelectionProperty) {
                std::string propertyValue = channel.getPropertySelectionValue(toStdString(property.getName().toString()));
                jsonValue[deviceName][channelName][propertyName] = propertyValue;
                // std::cout << "Property: " << property.getName();
                // std::cout << " " << isSelectionProperty << std::endl;
                // std::cout << "ValueType: " << property.getValueType() << std::endl;
            }
            else {
                std::string propertyValue = channel.getPropertyValue(toStdString(property.getName().toString()));
                if(property.getValueType() == 0 || property.getValueType() == 2)
                    jsonValue[deviceName][channelName][propertyName] = propertyValue;
                // std::cout << "Property: " << property.getName();
                // std::cout << " " << isSelectionProperty << std::endl;
                // std::cout << "ValueType: " << property.getValueType() << std::endl;
            }
            // jetPeer->notifyState(jetStatePath, jsonValue);
        }
    }
    // jetPeer->notifyState(jetStatePath, jsonValue);
}

void JetServer::printSomething()
{
    deviceLoop();
}

void JetServer::publishJetState()
{
    daq::jet::PeerAsync jetPeer(jet_eventloop, daq::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);

    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };

    Json::Value jetState;
    jetPeer.addStateAsync(jetStatePath, jetState, daq::jet::responseCallback_t(), cb);


    std::string deviceName = device.getName();
    auto properties = device.getAllProperties();
    for(auto property : properties) {
        std::string propertyName = property.getName();
        auto propertyValue = device.getPropertyValue(propertyName);
        jetState[deviceName][propertyName] = toStdString(propertyValue.toString());
    }

    std::vector<std::string> myVec = {"channel1", "channel2"};
    std::string name = myVec[0];
    auto channels = device.getChannels();
    for(auto channel : channels) {
        std::string channelName = channel.getName();
        std::cout << channelName << std::endl;
        auto channelProperties = channel.getAllProperties();
        std::cout << channelProperties.getCount() << std::endl;

        // jetState[deviceName].append()

        for(auto property : channelProperties) {
            std::string propertyName = property.getName();
            // if(propertyName == "DC")
            //     break;
            std::cout << propertyName << " " << property.getValueType() << " ";
            auto k = property.getSelectionValues().assigned();
            std::cout << "\n---- property.getSelectionValues().assigned() = " << property.getSelectionValues().assigned() << std::endl;
            auto propertyValue = channel.getPropertyValue(propertyName);
            jetState[deviceName][name][propertyName] = toStdString(propertyValue.toString());
            std::cout << name << std::endl;
        }
        name = myVec[1];
        std::cout << std::endl;
    }
    
    jetPeer.notifyState(jetStatePath, jetState);

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