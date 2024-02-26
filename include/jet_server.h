/*
 * Copyright 2022-2023 Blueberry d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <thread>
#include "common.h"
#include <opendaq/device_impl.h>
#include "hbk/sys/eventloop.h"
#include "hbk/string/replace.h"
#include "jet/peerasync.hpp"
#include "jet_server_base.h"

BEGIN_NAMESPACE_JET_MODULE

class JetServer : public JetServerBase
{
public:
    explicit JetServer(const DevicePtr& device);
    ~JetServer();
    void publishJetStates();

private:
    // Tree structure parsers and Jet state preparators
    void parseFolder(const FolderPtr& parentFolder);
    void prepareComponentJetState(const ComponentPtr& component);
    void parseComponentProperties(const ComponentPtr& component, Json::Value& parentJsonValue);
    template <typename ObjectType>
    void parseComponentSignals(const ObjectType& object, Json::Value& parentJsonValue);
    void prepareSignalJetState(const SignalPtr& signal);
    void parseComponentInputPorts(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue);
    void prepareInputPortJetState(const InputPortPtr& inputPort);

    // Defines callback function for an openDAQ component
    void createComponentCallback(const ComponentPtr& component);

    // Publishes an openDAQ component as a Jet state
    void publishComponentJetState(const std::string& path, const Json::Value& jetState);

    // Helper function which determines type of an openDAQ property
    template <typename PropertyHolder>
    void determinePropertyType(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

    // Creates a callable Jet method, which diverts the call to corresponding openDAQ function or procedure
    void createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property);

    // ********************************************************************************************************************************************
    //                          Functions which append different types of information of openDAQ objects to Jet states
    // ********************************************************************************************************************************************
    // Append properties to Json value
    template<typename PropertyHolderType, typename DataType>
    void appendSimpleProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendListProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendObjectProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendRatioProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendComplexNumber(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template<typename PropertyHolderType>
    void appendStructProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

    // Append other objects to Json value
    void appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue);
    void appendDeviceDomain(const DevicePtr& device, Json::Value& parentJsonValue);
    void appendFunctionBlockInfo(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue);

    // Append common metadata to Json value
    void appendGlobalId(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendVisibleStatus(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendTags(const ComponentPtr& component, Json::Value& parentJsonValue);


    // ********************************************************************************************************************************************
    //                                          Update functions addressing change events from openDAQ
    //!    These functions are also called when change is requested from Jet. This happens in order to update appropriate Jet state as well
    // ********************************************************************************************************************************************

    // Update Jet state attributes
    void OPENDAQ_EVENT_updateProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    template <typename DataType>
    void OPENDAQ_EVENT_updateSimpleProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void OPENDAQ_EVENT_updateListProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void OPENDAQ_EVENT_updateDictProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void OPENDAQ_EVENT_updateActiveStatus(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);

    // ********************************************************************************************************************************************
    //                              Update functions addressing change events from Jet (e.g. using "jetset" tool)
    // ********************************************************************************************************************************************
    void JET_EVENT_updateProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newPropertyValue);
    template <typename DataType>
    void JET_EVENT_updateSimpleProperty(const ComponentPtr& component, const std::string& propertyName, const DataType& newPropertyValue);
    void JET_EVENT_updateListProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonArray);
    void JET_EVENT_updateDictProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonDict);
    void JET_EVENT_updateActiveStatus(const ComponentPtr& component, const Json::Value& newActiveStatus);


    DevicePtr rootDevice; // Pointer to the root openDAQ device whose tree structure is parsed in order to publish it as Jet states
    hbk::jet::PeerAsync* jetPeer;

    hbk::sys::EventLoop jetEventloop;
    bool jetEventloopRunning;
    std::thread jetEventloopThread;
    void startJetEventloop();
    void stopJetEventloop();
    void startJetEventloopThread();
};


END_NAMESPACE_JET_MODULE