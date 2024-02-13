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
#include <jet_server.h>
#include <jet/peerasync.hpp>
#include <jet/peer.hpp>
#include <json/value.h>
#include <json/writer.h>


using namespace daq;
using namespace daq::modules::jet_module;
using namespace hbk::jet;


/******************************************************************************************
 *                                Function prototypes
*******************************************************************************************/

Json::Value readJetStates();
static void readJetStatesCb(std::promise<Json::Value>& promise, const Json::Value& value);
std::vector<std::string> getJetStatePaths(const Json::Value& jetStates);
std::vector<std::string> getComponentIDs(const FolderPtr& parentFolder);
void parseFolder(const FolderPtr& parentFolder, std::vector<std::string>& globalIdVector);
void modifyJetState(const char* valueType, const std::string& path, const char* newValue);


// Used to read Jet states from PeerAsync
static hbk::sys::EventLoop eventloop;

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
    std::string rootDevicePath;

    virtual void SetUp() {
        instance = daq::Instance(MODULE_PATH);
        instance.setRootDevice("daqref://device0");
        rootDevice = instance.getRootDevice();
        jetServer = new JetServer(rootDevice);
        rootDevicePath = toStdString(rootDevice.getGlobalId());
    }

    virtual void TearDown() {
        delete jetServer;
    }

    Json::Value getPropertyValueInJet(const std::string& propertyName, const Json::Value& defaultValue);
    Json::Value getPropertyListInJet(const std::string& propertyName);
    void setPropertyValueInJet(const std::string& propertyName, const Json::Value& newValue);
    void setPropertyListInJet(const std::string& propertyName, const std::vector<std::string>& newValue);
};

/**
 * @brief Gets a property value from a Jet state.
 * 
 * @param propertyName Name of the property.
 * @param defaultValue Return value if the property is not found in Jet.
 * @return Json::Value object which contains the property value in the Jet state.
 */
Json::Value JetServerTest::getPropertyValueInJet(const std::string& propertyName, const Json::Value& defaultValue)
{
    Json::Value jetStates = readJetStates();
    for (const Json::Value& item : jetStates) {
        if (item[hbk::jet::PATH] == rootDevicePath) {
            Json::Value valueInJet = item[hbk::jet::VALUE].get(propertyName, defaultValue);
            return valueInJet;
        }
    }
    return defaultValue; // Return default if not found
}

/**
 * @brief Gets a list property value from a Jet state.
 * 
 * @param propertyName Name of the property.
 * @return Json::Value object which contains the property value in the Jet state.
 */
Json::Value JetServerTest::getPropertyListInJet(const std::string& propertyName)
{
    Json::Value jetStates = readJetStates();
    for (const Json::Value& item : jetStates) {
        if (item[hbk::jet::PATH] == rootDevicePath) {
            Json::Value valueInJet = item[hbk::jet::VALUE];
            return valueInJet;
        }
    }
    return Json::Value(); // Return empty json if not found
}

/**
 * @brief Sets a property value in a Jet state.
 * 
 * @param propertyName Name of the property.
 * @param newValue Value to be applied to the property.
 */
void JetServerTest::setPropertyValueInJet(const std::string& propertyName, const Json::Value& newValue)
{
    Json::Value root;
    root[propertyName] = newValue;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDevicePath, jsonValue.c_str());
}

/**
 * @brief Sets a list property value in a Jet state.
 * 
 * @param propertyName Name of the property.
 * @param newValue Value to be applied to the property.
 */
void JetServerTest::setPropertyListInJet(const std::string& propertyName, const std::vector<std::string>& newValue)
{
    Json::Value root;
    Json::Value array(Json::arrayValue);
    for (auto val : newValue) {
        array.append(val);
    }
    root[propertyName] = array;
    Json::StreamWriterBuilder builder;
    const std::string jsonValue = Json::writeString(builder, root);
    modifyJetState("json", rootDevicePath, jsonValue.c_str());
}

/**
 * @brief Reads all of the published Jet states.
 * 
 * @return Object containg Jet states.
 */
Json::Value readJetStates()
{
    std::string address("127.0.0.1");
    unsigned int port = hbk::jet::JETD_TCP_PORT;
    hbk::jet::matcher_t match;
    hbk::jet::PeerAsync peer(eventloop, address, port);

    // Create a promise and future
    std::promise<Json::Value> promise;
    std::future<Json::Value> future = promise.get_future();

    // Calls the callback function with the promise
    peer.getAsync(match, [&promise](const Json::Value& value) {
        readJetStatesCb(promise, value);
    });

    eventloop.execute();

    // Wait for the future to get the value
    Json::Value result = future.get();
    return result;
}

/**
 * @brief Callback function for readJetStates function. It gets Jet states from PeerAsync.
 * 
 * @param promise Value which will be set once Jet states are read from PeerAsync.
 * @param value Json::Value which contains Jet states.
 */
static void readJetStatesCb(std::promise<Json::Value>& promise, const Json::Value& value)
{
    // value contains the data as an array of objects
    Json::Value publishedJsonValue = value[hbk::jsonrpc::RESULT];
    promise.set_value(publishedJsonValue);

    // Stop the event loop
    eventloop.stop();
}

/**
 * @brief Gets the path of the Jet states.
 * 
 * 
 * @param jetStates Json::Value objects which contain whole Jet states.
 * @return Vector containing paths of the Jet states.
 */
std::vector<std::string> getJetStatePaths(const Json::Value& jetStates)
{
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
std::vector<std::string> getComponentIDs(const FolderPtr& parentFolder)
{
    // Vector which is filled with global IDs of components
    std::vector<std::string> globalIdVector;

    // We have to add global ID of the root device manually as it's not added when it is parsed
    globalIdVector.push_back(parentFolder.asPtrOrNull<IDevice>().getGlobalId());
    // Get IDs of the components under the device
    parseFolder(parentFolder, globalIdVector);

    return globalIdVector;
}

/**
 * @brief Parses openDAQ folder to fill a vector of string with Global IDs of components under it.
 * 
 * @param parentFolder Root device, which is parsed to retrieve all objects under it.
 * @param globalIdVector Vector of strings which is filled with Global IDs.
 */
void parseFolder(const FolderPtr& parentFolder, std::vector<std::string>& globalIdVector)
{
    auto items = parentFolder.getItems();
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto component = item.asPtrOrNull<IComponent>();

       if (channel.assigned())
        {
            std::string globalId = channel.getGlobalId();
            globalIdVector.push_back(globalId);
        }
        else if (folder.assigned()) // It is important to test for folder last as a channel also is a folder!
        {
            parseFolder(folder, globalIdVector); // Folders are recursively parsed until non-folder items are identified in them
        }
        else if (component.assigned())  // It is important to test for component after folder!
        {
            std::string globalId = component.getGlobalId();
            globalIdVector.push_back(globalId);
        }
    }
}

/**
 * @brief Modifies Jet state.
 * 
 * @param valueType Type of the value that has to be modified. As all of the Jet states besides the methods are published with Json types,
 * use "json" as an argument.
 * @param path Path of the Jet state that is needed to be modified.
 * @param newValue New value of the Jet state.
 */
void modifyJetState(const char* valueType, const std::string& path, const char* newValue)
{
    unsigned int port = hbk::jet::JETD_TCP_PORT;
    std::string address("127.0.0.1");
    hbk::jet::Peer peer(address, port);
    // hbk::jet::PeerAsync peer(eventloop, address, port);
    if(strcmp(valueType, "bool") == 0) 
    {
        if(strcmp(newValue, "false") == 0)
        {
            peer.setStateValue(path, false, 2.71828182846);
        } 
        else if (strcmp(newValue, "true") == 0) 
        {
            peer.setStateValue(path, true, 2.71828182846);
        } 
        else 
        {
            std::cerr << "invalid value for boolean expecting 'true'', or 'false'" << std::endl;
        }
    } 
    else if(strcmp(valueType,"int")==0) 
    {
        int value = atoi(newValue);
        peer.setStateValue(path, value, 2.71828182846);
    } 
    else if(strcmp(valueType, "double")==0) 
    {
        double value = strtod(newValue, nullptr);
        peer.setStateValue(path, value, 2.71828182846);
    } 
    else if(strcmp(valueType,"string")==0) 
    {
        peer.setStateValue(path, newValue, 2.71828182846);
    } 
    else if(strcmp(valueType,"json")==0) 
    {
        Json::Value params;

        Json::CharReaderBuilder rBuilder;
        if(rBuilder.newCharReader()->parse(newValue, newValue+strlen(newValue), &params, nullptr)) 
        {
            peer.setStateValue(path, params, 2.71828182846);
        } 
        else 
        {
            std::cerr << "error while parsing json!" << std::endl;
        }
    }
}