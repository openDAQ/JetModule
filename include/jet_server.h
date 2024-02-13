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
    void parseFolder(const FolderPtr& parentFolder);

    void updateJetState(const PropertyObjectPtr& propertyObject);
    void updateJetState(const ComponentPtr& component);

    void createComponentJetState(const ComponentPtr& component);
    void createComponentListJetStates(const ListPtr<ComponentPtr>& componentList);

    template <typename PropertyHolder>
    void createJsonProperty(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    void createJsonProperties(const ComponentPtr& component);
    template <typename ValueType>
    void appendPropertyToJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value);
    void addJetState(const std::string& path);


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

    DevicePtr rootDevice;
    DictPtr<IString, IComponent> componentIdDict;

    Json::Value jsonValue;
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