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
    StringPtr getJetStatePath();
    void setJetStatePath(StringPtr path);

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
    template <typename ItemType>
    void appendListPropertyToJsonValue(const ComponentPtr& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue);
    void appendMetadataToJsonValue(const ComponentPtr& component, Json::Value& parentJsonValue);
    void addJetState(const std::string& path);

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

    StringPtr jetStatePath = "/daq/JetModule/";
};


END_NAMESPACE_JET_MODULE