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
#include <opendaq/device_impl.h>
#include <json/value.h>

enum JetModuleException : int
{
    JM_INCOMPATIBLE_TYPES = 0,
    JM_UNSUPPORTED_JSON_TYPE,
    JM_UNSUPPORTED_DAQ_TYPE,
    JM_UNSUPPORTED_ITEM
};

bool checkTypeCompatibility(Json::ValueType jsonValueType, daq::CoreType daqValueType);
void throwJetModuleException(JetModuleException jmException);
void throwJetModuleException(JetModuleException jmException, std::string propertyName);
void throwJetModuleException(JetModuleException jmException, Json::ValueType jsonValueType, std::string propertyName, std::string globalId);