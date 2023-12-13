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

BEGIN_NAMESPACE_JET_MODULE

enum JetModuleException : int
{
    JM_INCOMPATIBLE_TYPES = 0,
    JM_UNSUPPORTED_JSON_TYPE,
    JM_UNSUPPORTED_DAQ_TYPE
};

class JetServer
{
public:
    static constexpr char jetStatePath[] = "/daq/JetModule/";
    static constexpr char globalIdString[] = "Global ID";
    static constexpr char typeString[] = "_type";

    explicit JetServer(const DevicePtr& device);
    ~JetServer();
    void publishJetStates();
    void updateJetState(const PropertyObjectPtr& propertyObject);

private:
    void createComponentJetState(const ComponentPtr& component);
    void createComponentListJetStates(const ListPtr<ComponentPtr>& componentList);

    template <typename PropertyHolder>
    void createJsonProperty(const ComponentPtr& propertyPublisher, const PropertyPtr& property, const PropertyHolder& propertyHolder);
    void createJsonProperties(const ComponentPtr& component);
    template <typename ValueType>
    void appendPropertyToJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value);
    template <typename ItemType>
    void appendListPropertyToJsonValue(const ComponentPtr& propertyHolder, const PropertyPtr& property);
    void appendMetadataToJsonValue(const ComponentPtr& component);
    void addJetState(const std::string& path);

    bool determineSelectionProperty(const PropertyPtr& property);

    bool propertyCallbacksCreated;
    void createCallbackForProperty(const PropertyPtr& property);
    
    void convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index);
    void createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property);

    DevicePtr rootDevice;
    DictPtr<IString, IComponent> componentIdDict;

    Json::Value jsonValue;
    hbk::jet::PeerAsync* jetPeer;

    hbk::sys::EventLoop jetEventloop;
    bool jetEventloopRunning;
    std::thread jetEventloopThread;
    void startJetEventloop();
    void stopJetEventloop();
    void startJetEventloopThread();

    ListPtr<BaseObjectPtr> convertJsonToDaqArray(const ComponentPtr& propertyHolder, const std::string& propertyName, const Json::Value& value);

    bool checkTypeCompatibility(Json::ValueType jsonValueType, daq::CoreType daqValueType);
    void throwJetModuleException(JetModuleException jmException);
    void throwJetModuleException(JetModuleException jmException, Json::ValueType jsonValueType, std::string propertyName, std::string globalId);
};


END_NAMESPACE_JET_MODULE