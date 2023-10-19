#pragma once
#include "common.h"
#include <opendaq/device_impl.h>

#include "daq/sys/eventloop.h"
#include "daq/string/replace.h"
#include "jet/peerasync.hpp"

BEGIN_NAMESPACE_JET_MODULE

#define SELECTION_PROPERTY_RETURN_VALUE 0xAA // randomly chosen value that determinePropertyType() returns when it encounters SelectionProperty

class JetServer
{
public:
    static constexpr char jetStatePath[] = "/daq/jetModule";
    static constexpr char rootDeviceType[] = "Root Device";
    static constexpr char deviceType[] = "Device";
    static constexpr char channelType[] = "Channel";
    static constexpr char functionBlockType[] = "Function Block";
    static constexpr char customComponentType[] = "Custom Component";

    explicit JetServer(DevicePtr device);
    void publishJetState();
private:
    void parseRootDeviceProperties();
    void parseDeviceProperties();
    void parseChannelProperties();
    void parseFunctionBlockProperties();
    void parseCustomComponentProperties();

    void createJsonProperties(PropertyObjectPtr propertyObject, ConstCharPtr objectType);
    template <typename ValueType>
    void appendJsonValue(ConstCharPtr objectType, std::string propertyName, ValueType value);

    bool determineSelectionProperty(PropertyPtr property);
    CoreType determinePropertyType(PropertyPtr property);

    DevicePtr rootDevice;
    std::string rootDeviceName;
    std::string deviceName;
    std::string channelName;
    std::string functionBlockName;
    std::string customComponentName;

    Json::Value jsonValue;
    daq::sys::EventLoop jet_eventloop; 
    daq::jet::PeerAsync* jetPeer;
};


END_NAMESPACE_JET_MODULE