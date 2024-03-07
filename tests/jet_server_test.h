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
#include <opendaq/opendaq.h>
#include <jet/peerasync.hpp>
#include <jet/peer.hpp>
#include <json/value.h>
#include <json/writer.h>
#include "jet_server.h"
#include "jet_peer_wrapper.h"
#include "property_converter.h"
#include "jet_event_handler.h"


#define JET_GET_VALUE_TIMEOUT (1) // 1 second

using namespace daq;
using namespace daq::modules::jet_module;
using namespace hbk::jet;

/**
 * @brief Creates openDAQ instance using device from openDAQ's 'ref_device_module'.
 * Creates a JetServer object which can be used to publish openDAQ device structure as Jet states.
 * 
 */
class JetServerTest : public ::testing::Test {
protected:
    daq::InstancePtr instance;
    daq::DevicePtr rootDevice;
    JetServer* jetServer;
    JetPeerWrapper& jetPeerWrapper = JetPeerWrapper::getInstance();
    PropertyConverter propertyConverter;
    JetEventHandler jetEventHandler;
    std::string rootDevicePath;

    virtual void SetUp() {
        instance = daq::Instance(MODULE_PATH);
        instance.setRootDevice("daqref://device0");
        rootDevice = instance.getRootDevice();
        jetServer = new JetServer(instance);
        jetServer->publishJetStates();

        rootDevicePath = toStdString(rootDevice.getGlobalId());
    }

    virtual void TearDown() {
        delete jetServer;
    }

    Json::Value getPropertyValueInJet(const std::string& propertyName);
    Json::Value getPropertyValueInJetTimeout(const std::string& propertyName, const Json::Value& expectedValue);
    void setPropertyValueInJet(const std::string& propertyName, const Json::Value& newValue);
    void setPropertyListInJet(const std::string& propertyName, const std::vector<std::string>& newValue);

    std::vector<std::string> getComponentIDs();
    std::vector<std::string> getJetStatePaths();
    void parseOpendaqInstance(const FolderPtr& parentFolder, std::vector<std::string>& globalIdVector);
};

/**
 * @brief Gets a property value from a Jet state.
 * 
 * @param propertyName Name of the property.
 * @return Json::Value object which contains the property value in the Jet state.
 */
Json::Value JetServerTest::getPropertyValueInJet(const std::string& propertyName)
{
    Json::Value jetState = jetPeerWrapper.readJetState(rootDevice.getGlobalId());
    Json::Value valueInJet = jetState.get(propertyName, Json::Value()); // default value is empty Json
    return valueInJet;
}

/**
 * @brief Gets a property value from a Jet state. Expected value is passed to repeteadly read a Jet state for a given time. This is needed
 * in cases where value change in Jet needs some time to be propagated.
 * 
 * @param propertyName Name of the property.
 * @param expectedValue Value expected to be read from a Jet state.
 * @return Json::Value object which contains the property value in the Jet state.
 */
Json::Value JetServerTest::getPropertyValueInJetTimeout(const std::string& propertyName, const Json::Value& expectedValue)
{
    Json::Value valueInJet;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto timeout = std::chrono::seconds(JET_GET_VALUE_TIMEOUT);
    do {
        valueInJet = getPropertyValueInJet(propertyName);
        if (valueInJet == expectedValue) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait a bit before retrying
    } while (std::chrono::high_resolution_clock::now() - startTime < timeout);

    return valueInJet;
}

/**
 * @brief Sets a property value in a Jet state.
 * 
 * @param propertyName Name of the property.
 * @param newValue Value to be applied to the property.
 */
void JetServerTest::setPropertyValueInJet(const std::string& propertyName, const Json::Value& newValue)
{
    jetEventHandler.updateProperty(rootDevice, propertyName, newValue);
}

/**
 * @brief Sets a list property value in a Jet state.
 * 
 * @param propertyName Name of the property.
 * @param newValue Value to be applied to the property.
 */
void JetServerTest::setPropertyListInJet(const std::string& propertyName, const std::vector<std::string>& newValue)
{
    Json::Value array(Json::arrayValue);
    for (auto val : newValue) {
        array.append(val);
    }
    jetEventHandler.updateProperty(rootDevice, propertyName, array);
}

/**
 * @brief Gets the path of the Jet states.
 * 
 * 
 * @param jetStates Json::Value objects which contain whole Jet states.
 * @return Vector containing paths of the Jet states.
 */
std::vector<std::string> JetServerTest::getJetStatePaths()
{
    Json::Value jetStates = jetPeerWrapper.readAllJetStates();
    // Vector which will be filled with paths of Jet states
    std::vector<std::string> jetStatePaths;
    for (const Json::Value &item : jetStates) {
        jetStatePaths.push_back(item[hbk::jet::PATH].asString());
    }

    return jetStatePaths;
}

/**
 * @brief Gets Global IDs of openDAQ components.
 * 
 * @param parentFolder Root device, which is parsed to retrieve all objects under it.
 * @return Vector containing Global IDs of openDAQ components.
 */
std::vector<std::string> JetServerTest::getComponentIDs()
{
    // Vector which is filled with global IDs of components
    std::vector<std::string> globalIdVector;

    // We have to add global ID of the root device manually as it's not added when it is parsed
    globalIdVector.push_back(rootDevice.getGlobalId());
    // Get IDs of the components under the device
    parseOpendaqInstance(instance, globalIdVector);

    return globalIdVector;
}

/**
 * @brief Parses openDAQ folder to fill a vector of string with Global IDs of components under it.
 * 
 * @param parentFolder Root device, which is parsed to retrieve all objects under it.
 * @param globalIdVector Vector of strings which is filled with Global IDs.
 */
void JetServerTest::parseOpendaqInstance(const FolderPtr& parentFolder, std::vector<std::string>& globalIdVector)
{
    auto items = parentFolder.getItems(search::Any());
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto component = item.asPtrOrNull<IComponent>();
        auto device = item.asPtrOrNull<IDevice>();
        auto functionBlock = item.asPtrOrNull<IFunctionBlock>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto signal = item.asPtrOrNull<ISignal>();
        auto inputPort = item.asPtrOrNull<IInputPort>();

        if(device.assigned()) {
            globalIdVector.push_back(device.getGlobalId());
        }
        else if(channel.assigned()) {
            globalIdVector.push_back(channel.getGlobalId());
        }
        else if(functionBlock.assigned()) {
            globalIdVector.push_back(functionBlock.getGlobalId());
        }
        else if(signal.assigned()) {
            globalIdVector.push_back(signal.getGlobalId());
        }
        else if(inputPort.assigned()) {
            globalIdVector.push_back(inputPort.getGlobalId());
        }
        else if(folder.assigned()) { // It is important to test for folder last as everything besides component is a folder as well
            // We do nothing here because we want to identify pure components (not its descendants)
            // Recursion is done in separate if statement
        }
        else if(component.assigned()) { // It is important to test for component after folder!
            globalIdVector.push_back(component.getGlobalId());
        }
        else {
            // throwJetModuleException(JM_UNSUPPORTED_ITEM);
        }

        if(folder.assigned()) {
            parseOpendaqInstance(folder, globalIdVector);
        }
    }
}

Json::Value convertVectorToJson(const std::vector<std::string>& vector)
{
    Json::Value array(Json::arrayValue);
    for (auto val : vector) {
        array.append(val);
    }
    return array;
}