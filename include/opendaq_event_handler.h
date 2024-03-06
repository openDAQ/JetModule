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

class OpendaqEventHandler
{
public:
    OpendaqEventHandler();

    //  Update functions addressing change events from openDAQ
    //! These functions are also called when change is requested from Jet. This happens in order to update appropriate Jet state as well
    void updateProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    template <typename DataType>
    void updateSimpleProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void updateListProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void updateDictProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);
    void updateActiveStatus(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters);

private:
    JetPeerWrapper& jetPeerWrapper;
    PropertyConverter propertyConverter;

    LoggerComponentPtr logger;
};

END_NAMESPACE_JET_MODULE