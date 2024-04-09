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
#include <json/value.h>
#include <opendaq/device_impl.h>

using namespace daq;

BEGIN_NAMESPACE_JET_MODULE

class PropertyConverter
{
public:
    explicit PropertyConverter();

    ListPtr<IBaseObject> convertJsonArrayToOpendaqList(const Json::Value& jsonArray);
    DictPtr<IString, IBaseObject> convertJsonDictToOpendaqDict(const Json::Value& jsonDict);
    PropertyObjectPtr convertJsonObjectToOpendaqObject(const Json::Value& jsonObject, const std::string& pathPrefix);

    Json::Value convertOpendaqListToJsonArray(const ListPtr<IBaseObject>& opendaqList, const CoreType& listItemType);
    Json::Value convertOpendaqDictToJsonDict(const DictPtr<IString, IBaseObject>& opendaqDict, const CoreType& dictItemType);

    Json::Value convertDataRuleToJsonObject(const DataRulePtr& dataRule);

    void convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index);

private:
    template <typename ListItemType>
    Json::Value fillJsonArray_BasicType(const ListPtr<IBaseObject>& opendaqList);
    Json::Value fillJsonArray_Ratio(const ListPtr<IBaseObject>& opendaqList);
    Json::Value fillJsonArray_ComplexNumber(const ListPtr<IBaseObject>& opendaqList);

    template <typename DictItemType>
    Json::Value fillJsonDict_BasicType(const DictPtr<IString, IBaseObject>& opendaqDict);
};

END_NAMESPACE_JET_MODULE