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
#include "common.h"
#include <opendaq/instance_ptr.h>
#include <opendaq/component_ptr.h>
#include <opendaq/folder_ptr.h>
#include "property_manager.h"
#include "property_converter.h"
#include "jet_peer_wrapper.h"
#include "opendaq_event_handler.h"
#include "jet_event_handler.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Converter of openDAQ components into Json representations of them. This class is parent of all other types of converters.
 * It holds function for appending various metadata information to Json values which later will be published as Jet states.
 * Here are the functions which define callbacks when something is changed from openDAQ or Jet.
 * 
 */
class ComponentConverter
{
public:
    explicit ComponentConverter(const InstancePtr& opendaqInstance);

    virtual void composeJetState(const ComponentPtr& component);

protected:
    void createOpendaqCallback(const ComponentPtr& component);
    JetStateCallback createJetCallback();
    JetStateCallback createObjectPropertyJetCallback();

    void appendProperties(const ComponentPtr& component, Json::Value& parentJsonValue);

    // Append common metadata to Json value
    void appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendVisibleStatus(const ComponentPtr& component, Json::Value& parentJsonValue);
    void appendTags(const ComponentPtr& component, Json::Value& parentJsonValue);

    JetPeerWrapper& jetPeerWrapper;
    PropertyManager propertyManager;
    PropertyConverter propertyConverter;
    OpendaqEventHandler opendaqEventHandler;
    JetEventHandler jetEventHandler;

    InstancePtr opendaqInstance;
    LoggerComponentPtr logger;
private:
};

END_NAMESPACE_JET_MODULE