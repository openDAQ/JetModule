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
    static constexpr char jetStatePath[] = "/daq/JetModule/";
    static constexpr char globalIdString[] = "Global ID";
    static constexpr char typeString[] = "_type";

    explicit JetServer(const DevicePtr& device);
    void publishJetStates();
    void updateJetState(const ComponentPtr& component, const std::string& propertyName);

private:
    void createComponentJetState(const ComponentPtr& component);
    void createComponentListJetStates(const ListPtr<ComponentPtr>& componentList);

    void createJsonProperty(const ComponentPtr& component, const PropertyPtr& property);
    void createJsonProperties(const ComponentPtr& component);
    template <typename ValueType>
    void appendJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value);
    void addJetState(const std::string& path);

    bool determineSelectionProperty(const PropertyPtr& property);

    DevicePtr rootDevice;
    std::string rootDeviceName;
    std::string deviceName;
    std::string channelName;
    std::string functionBlockName;
    std::string customComponentName;
    std::string signalName;

    Json::Value jsonValue;
    daq::sys::EventLoop jet_eventloop; 
    daq::jet::PeerAsync* jetPeer;
};


END_NAMESPACE_JET_MODULE