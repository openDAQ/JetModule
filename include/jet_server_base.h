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
#include <future>
#include <json/value.h>
#include <opendaq/device_impl.h>
#include <opendaq/logger_component_factory.h>
#include <jet/peerasync.hpp>
#include "common.h"
#include "jet_module_exceptions.h"

using namespace daq;

BEGIN_NAMESPACE_JET_MODULE

class JetServerBase
{
public:
    explicit JetServerBase();

    void convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index);

    static Json::Value readJetState(const std::string& path);
    void modifyJetState(const std::string& path, const std::string& entryName, const Json::Value& entryValue);
protected:
    std::string removeRootDeviceId(const std::string& path);

    LoggerComponentPtr logger;
private:
    static void readJetStateCb(std::promise<Json::Value>& promise, const Json::Value& value);
};

END_NAMESPACE_JET_MODULE