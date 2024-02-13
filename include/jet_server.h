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
    // Tree structure parsers of an openDAQ device
    void parseFolder(const FolderPtr& parentFolder);
    void prepareComponentJetState(const ComponentPtr& component);
    void parseComponentProperties(const ComponentPtr& component);

    // Tree structure publisher of an openDAQ device
    void publishComponentJetState(const std::string& path);

    // Functions which are called when some component's property is updated from Jet (e.g. using jetset tool)
    void updateJetState(const PropertyObjectPtr& propertyObject);
    void updateJetState(const ComponentPtr& component);

    // Helper function which determines type of an openDAQ property
    template <typename PropertyHolder>
    void determinePropertyType(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

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

    // List and Dict property helper functions
    template <typename PropertyHolderType, typename ItemType>
    void fillListProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template <typename PropertyHolderType>
    void fillListPropertyWithRatio(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue);
    template <typename PropertyHolderType, typename KeyType>
    void determineDictItemType(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    template <typename PropertyHolderType, typename KeyType, typename ItemType>
    void fillDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);

    // Append other objects to Json value
    void appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue);
    void appendDeviceDomain(const DevicePtr& device, Json::Value& parentJsonValue);
    void appendFunctionBlockInfo(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue);
    void appendInputPorts(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue);
    template <typename ObjectType>
    void appendOutputSignals(const ObjectType& object, Json::Value& parentJsonValue);

    // Append common metadata to Json value
    void appendGlobalId(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendTags(const ComponentPtr& component, Json::Value& parentJsonValue);

    bool propertyCallbacksCreated;
    void createCallbackForProperty(const PropertyPtr& property);
    
    void createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property);

    DevicePtr rootDevice; // Pointer to the root openDAQ device whose tree structure is parsed in order to publish it as Jet states
    DictPtr<IString, IComponent> componentIdDict; // Stores global IDs of components and their pointers
    Json::Value jsonValue; // Container which is filled for every component and is cleared when the component's tree structure is published as a Jet state

    hbk::jet::PeerAsync* jetPeer;

    bool jetStateUpdateDisabled;

    hbk::sys::EventLoop jetEventloop;
    bool jetEventloopRunning;
    std::thread jetEventloopThread;
    void startJetEventloop();
    void stopJetEventloop();
    void startJetEventloopThread();
};


END_NAMESPACE_JET_MODULE