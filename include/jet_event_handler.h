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
#include <opendaq/component_ptr.h>
#include "jet_peer_wrapper.h"
#include "property_converter.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Handler of events occured in Jet states. Functions in this class update openDAQ components based on changes in their corresponding
 * Jet states.
 * 
 */
class JetEventHandler
{
public:
    JetEventHandler();

    // Update functions addressing change events from Jet (e.g. using "jetset" tool)
    void updateProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newPropertyValue);
    template <typename DataType>
    void updateSimpleProperty(const ComponentPtr& component, const std::string& propertyName, const DataType& newPropertyValue);
    void updateListProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonArray);
    void updateDictProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonDict);
    void updateObjectProperty(const ComponentPtr& component, const Json::Value& newJsonObject);
    void updateActiveStatus(const ComponentPtr& component, const Json::Value& newActiveStatus);

private:
    // Helper functions
    std::vector<std::pair<std::string, Json::Value>> extractObjectPropertyPathsAndValues(const Json::Value& objectPropertyJetState);
    void extractObjectPropertyPathsAndValuesInternal(const Json::Value& objectPropertyJetState, const std::string& path, std::vector<std::pair<std::string, Json::Value>>& pathAndValuePairs);

    JetPeerWrapper& jetPeerWrapper;
    PropertyConverter propertyConverter;

    LoggerComponentPtr logger;
};

END_NAMESPACE_JET_MODULE