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

    explicit JetServer(DevicePtr device);
    void publishJetState();
private:
    void getDeviceProperties();
    void getChannelProperties();
    void createJsonProperties(PropertyObjectPtr propertyObject);
    template <typename ValueType>
    void appendJsonValue(ConstCharPtr propertyObjectType, std::string propertyName, ValueType value);
    void getFunctionBlockProperties();

    bool determineSelectionProperty(PropertyPtr property);
    CoreType determinePropertyType(PropertyPtr property);

    DevicePtr device;
    std::string deviceName;
    std::string channelName;

    Json::Value jsonValue;
    daq::sys::EventLoop jet_eventloop; 
    daq::jet::PeerAsync* jetPeer;
};


END_NAMESPACE_JET_MODULE