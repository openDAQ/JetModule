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
#include <opendaq/instance_ptr.h>
#include "component_converter.h"
#include "device_converter.h"
#include "function_block_converter.h"
#include "channel_converter.h"
#include "signal_converter.h"
#include "input_port_converter.h"

BEGIN_NAMESPACE_JET_MODULE

class JetServer
{
public:
    explicit JetServer(const InstancePtr& instance);
    ~JetServer();
    void publishJetStates();

protected:

private:
    void parseOpendaqInstance(const FolderPtr& parentFolder);

    InstancePtr opendaqInstance;
    DevicePtr rootDevice; // Pointer to the root openDAQ device whose tree structure is parsed in order to publish it as Jet states

    ComponentConverter componentConverter;
    DeviceConverter deviceConverter;
    FunctionBlockConverter functionBlockConverter;
    ChannelConverter channelConverter;
    SignalConverter signalConverter;
    InputPortConverter inputPortConverter;
};


END_NAMESPACE_JET_MODULE