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
#include <jet/peerasync.hpp>
#include "common.h"
#include "jet_module_exceptions.h"

using namespace daq;

BEGIN_NAMESPACE_JET_MODULE

// Defining the callback type which is called whenever Jet state is updated from Jet (e.g. via jetset tool)
using JetStateCallback  = std::function<Json::Value(const Json::Value&, const std::string&)>;
// Callback which is called when a Jet method is called
using JetMethodCallback = std::function<Json::Value(const Json::Value&)>;

//! This class has to be instantiated only once because PeerAsync occupies unix socket
//! Singleton pattern is utilized
/**
 * @brief Wrapper class which make communication with Jet easy. It has function for publishing, reading and modifying Jet states.
 * Singleton design pattern is utilized in this class because it has to be instantiated only once. Otherwise there would
 * be conflicts between Jet peers trying to occupy unix socket.
 * 
 */
class JetPeerWrapper
{
public:
    // Accessor for the JetPeerWrapper instance
    static JetPeerWrapper& getInstance() {
        static JetPeerWrapper instance; // Guaranteed to be destroyed and instantiated on first use.
        return instance;
    }

    void publishJetState(const std::string& path, const Json::Value& jetState, JetStateCallback callback);
    void publishJetMethod(const std::string& path, JetMethodCallback callback);
    Json::Value readJetState(const std::string& path);
    Json::Value readAllJetStates();
    void updateJetState(const std::string& path, const Json::Value newValue);
    void modifyJetState(const char* valueType, const std::string& path, const char* newValue);

    // Helper functions
    std::string removeRootDeviceId(const std::string& path);
    std::string removeObjectPropertyName(const std::string& path);

private:
    explicit JetPeerWrapper(); // Private constructor
    ~JetPeerWrapper();
    JetPeerWrapper(const JetPeerWrapper&) = delete; // Prevent copy-construction
    JetPeerWrapper& operator=(const JetPeerWrapper&) = delete; // Prevent assignment

    static void readJetStateCb(std::promise<Json::Value>& promise, const Json::Value& value);

    hbk::jet::PeerAsync* jetPeer;

    void startJetEventloop();
    void stopJetEventloop();
    void startJetEventloopThread();
    hbk::sys::EventLoop jetEventloop;
    bool jetEventloopRunning;
    std::thread jetEventloopThread;

    LoggerComponentPtr logger;
};

END_NAMESPACE_JET_MODULE