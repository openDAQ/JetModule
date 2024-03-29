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
#include "component_converter.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Converter of openDAQ devices into their Json representations.
 * 
 */
class DeviceConverter : public ComponentConverter
{
public:
    DeviceConverter(const InstancePtr& opendaqInstance) : ComponentConverter(opendaqInstance) {}
    void composeJetState(const ComponentPtr& component) override;

private:
    void appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue);
    void appendDeviceDomain(const DevicePtr& device, Json::Value& parentJsonValue);
};

END_NAMESPACE_JET_MODULE