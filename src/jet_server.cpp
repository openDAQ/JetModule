#include "jet_server.h"
#include "jet/defines.h"
#include <iostream>
#include <string>

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    propertyCallbacksCreated = false;
    jetEventloopRunning = false;

    startJetEventloopThread();
    jetPeer = new hbk::jet::PeerAsync(jetEventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
    componentIdDict = Dict<IString, IComponent>();
}

JetServer::~JetServer()
{
    stopJetEventloop();
    delete(jetPeer);
}

void JetServer::addJetState(const std::string& path)
{
    auto cb = [this](const Json::Value& value, std::string path) -> Json::Value
    {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        std::string globalId = value.get("Global ID", "").asString();
        ComponentPtr component = componentIdDict.get(globalId);
        auto properties = component.getAllProperties();
        for(auto property : properties)
        {
            std::string propertyName = property.getName();
            if (!value.isMember(propertyName))
            {
                std::cout << "addJetState: skipping property " << propertyName << std::endl;
                continue;
            }

            Json::ValueType jsonValueType = value.get(propertyName, "").type();
            bool typesAreCompatible = checkTypeCompatibility(jsonValueType, property.getValueType());
            if(!typesAreCompatible)
            {
                throwJetModuleException(JetModuleException::JM_INCOMPATIBLE_TYPES);
                continue;
            }

            std::cout << "addJetState: Changing value for property " << propertyName << ", type: " << jsonValueType << std::endl;
            try
            {
                switch(jsonValueType)
                {
                    case Json::ValueType::intValue:
                        {
                            int64_t oldValue = component.getPropertyValue(propertyName);
                            int64_t newValue = value.get(propertyName, "").asInt64(); 
                            if (oldValue != newValue)
                                component.setPropertyValue(propertyName, newValue);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    case Json::ValueType::uintValue:
                        {
                            uint64_t oldValue = component.getPropertyValue(propertyName);
                            uint64_t newValue = value.get(propertyName, "").asUInt64();
                            if (oldValue != newValue)
                                component.setPropertyValue(propertyName, newValue);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    case Json::ValueType::realValue:
                        {
                            double oldValue = component.getPropertyValue(propertyName);
                            double newValue = value.get(propertyName, "").asDouble();
                            if (oldValue != newValue)
                                component.setPropertyValue(propertyName, newValue);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    case Json::ValueType::stringValue:
                        {
                            std::string oldValue = component.getPropertyValue(propertyName);
                            std::string newValue = value.get(propertyName, "").asString();
                            if (oldValue != newValue)
                                component.setPropertyValue(propertyName, newValue);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    case Json::ValueType::booleanValue:
                        {
                            bool oldValue = component.getPropertyValue(propertyName);
                            bool newValue = value.get(propertyName, "").asBool();
                            if (oldValue != newValue)
                                component.setPropertyValue(propertyName, newValue);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    case Json::ValueType::arrayValue:
                        {
                            ListPtr<BaseObjectPtr> oldDaqArray = component.getPropertyValue(propertyName);
                            ListPtr<BaseObjectPtr> newDaqArray = convertJsonToDaqArray(component, propertyName, value);
                            if(oldDaqArray != newDaqArray)
                                component.setPropertyValue(propertyName, newDaqArray);
                            else std::cout << "Value for " << propertyName << " has not changed. Skipping.." << std::endl;
                        }
                        break;
                    default:
                        throwJetModuleException(JetModuleException::JM_UNSUPPORTED_JSON_TYPE, jsonValueType, propertyName, globalId);
                        break;
                }
            }
            catch(...)
            {
                throwJetModuleException(JetModuleException::JM_UNSUPPORTED_JSON_TYPE, jsonValueType, propertyName, globalId);
            }
        } // for
        std::cout << "Properties of " + globalId + " successfully updated" << std::endl;
        return Json::Value();
    };

    jetPeer->addStateAsync(path, jsonValue, hbk::jet::responseCallback_t(), cb);
    jsonValue.clear();
}

ListPtr<BaseObjectPtr> JetServer::convertJsonToDaqArray(const ComponentPtr& propertyHolder, const std::string& propertyName, const Json::Value& value)
{
    auto array = value.get(propertyName, "");
    uint64_t arraySize = array.size();
    Json::ValueType arrayElementType =  array[0].type();

    ListPtr<BaseObjectPtr> daqArray;
    switch(arrayElementType)
    {
        case Json::ValueType::nullValue:
            std::cout << "Null type element detected in the array" << std::endl;
            break;
        case Json::ValueType::intValue:
            daqArray = List<int>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asInt());
            }
            break;
        case Json::ValueType::uintValue:
            daqArray = List<uint>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asUInt());
            }
            break;
        case Json::ValueType::realValue:
            daqArray = List<double>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asDouble());
            }
            break;
        case Json::ValueType::stringValue:
            daqArray = List<std::string>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asString());
            }
            break;
        case Json::ValueType::booleanValue:
            daqArray = List<bool>();
            for(int i = 0; i < arraySize; i++) {
                daqArray.pushBack(array[i].asBool());
            }
            break;
        default:
            std::cout << "Unsupported array element type detected: " << arrayElementType << std::endl;
    }

    return daqArray;
}

void JetServer::updateJetState(const PropertyObjectPtr& propertyObject)
{
    ComponentPtr component = propertyObject.asPtr<IComponent>();
    createJsonProperties(component);   
    appendMetadataToJsonValue(component);

    std::string path = jetStatePath + component.getGlobalId();
    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::publishJetStates()
{
    createComponentJetState(rootDevice);
    auto devices = rootDevice.getDevices();
    createComponentListJetStates(devices);
    auto channels = rootDevice.getChannels();
    createComponentListJetStates(channels);
    auto functionBlocks = rootDevice.getFunctionBlocks();
    createComponentListJetStates(functionBlocks);
    auto customComponents = rootDevice.getCustomComponents();
    createComponentListJetStates(customComponents);
    auto signals = rootDevice.getSignals();
    createComponentListJetStates(signals);

    propertyCallbacksCreated = true;
}

void JetServer::createComponentJetState(const ComponentPtr& component)
{
    componentIdDict.set(component.getGlobalId(), component);
    createJsonProperties(component);   
    appendMetadataToJsonValue(component);
    std::string path = jetStatePath + component.getGlobalId();
    addJetState(path);
}   

void JetServer::createComponentListJetStates(const ListPtr<ComponentPtr>& componentList)
{
    for(auto component : componentList) {
        createComponentJetState(component);
    }
}

template <typename PropertyHolder>
void JetServer::createJsonProperty(const ComponentPtr& propertyPublisher, const PropertyPtr& property, const PropertyHolder& propertyHolder)
{
    bool isSelectionProperty = determineSelectionProperty(property);
    std::string propertyName = property.getName();
    bool propertyValueBool;
    int64_t propertyValueInt;
    double propertyValueFloat;
    StringPtr propertyValueString;
    CoreType listItemType;

    CoreType propertyType = property.getValueType();
    switch(propertyType) {
        case CoreType::ctBool:
            propertyValueBool = propertyHolder.getPropertyValue(propertyName);
            appendPropertyToJsonValue<bool>(propertyPublisher, propertyName, propertyValueBool);
            break;
        case CoreType::ctInt:
            propertyValueInt = propertyHolder.getPropertyValue(propertyName);
            appendPropertyToJsonValue<int64_t>(propertyPublisher, propertyName, propertyValueInt);
            break;
        case CoreType::ctFloat:
            propertyValueFloat = propertyHolder.getPropertyValue(propertyName);
            appendPropertyToJsonValue<double>(propertyPublisher, propertyName, propertyValueFloat);
            break;
        case CoreType::ctList:
            listItemType = property.getItemType();
            switch(listItemType)
            {
                case CoreType::ctBool:
                    appendListPropertyToJsonValue<bool>(propertyPublisher, property);
                    break;
                case CoreType::ctInt:
                    appendListPropertyToJsonValue<int>(propertyPublisher, property);
                    break;
                case CoreType::ctFloat:
                    appendListPropertyToJsonValue<float>(propertyPublisher, property);
                    break;
                case CoreType::ctString:
                    appendListPropertyToJsonValue<std::string>(propertyPublisher, property);
                    break;
                default:
                    std::cout << "Unsupported list item type: " << listItemType << std::endl;
            }
            break;
        case CoreType::ctString:
            propertyValueString = propertyHolder.getPropertyValue(propertyName);
            appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, toStdString(propertyValueString));
            break;
        case CoreType::ctProc:
            createJetMethod(propertyPublisher, property);
            break;
        case CoreType::ctFunc:
            createJetMethod(propertyPublisher, property);
            break;
        default:
            std::cout << "Unsupported value type \"" << propertyType << "\" of Property: " << propertyName << std::endl;
            std::cout << "\"std::string\" will be used to store property value." << std::endl;
            auto propertyValue = propertyHolder.getPropertyValue(propertyName);
            appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, propertyValue);
            break;
    }
}

void JetServer::createJsonProperties(const ComponentPtr& component)
{
    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty<ComponentPtr>(component, property, component);
        if(!propertyCallbacksCreated)
            createCallbackForProperty(property);
    }

    // Checking whether the component is a device. If it's a device we have to get deviceInfo properties manually
    if(strcmp(component.asPtr<ISerializable>().getSerializeId(), "Device") == 0) {
        auto deviceInfo = component.asPtr<IDevice>().getInfo();
        auto deviceInfoProperties = deviceInfo.getAllProperties();
        for(auto property : deviceInfoProperties) {
            createJsonProperty<DeviceInfoPtr>(component, property, deviceInfo);
            if(!propertyCallbacksCreated)
                createCallbackForProperty(property);
        }
    }
}

template <typename ValueType>
void JetServer::appendPropertyToJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value)
{
    std::string componentName = toStdString(component.getName());
    jsonValue[componentName][propertyName] = value;
}

template <typename ItemType>
void JetServer::appendListPropertyToJsonValue(const ComponentPtr& propertyHolder, const PropertyPtr& property)
{
    ListPtr<ItemType> propertyList = List<ItemType>();
    std::string componentName = propertyHolder.getName();
    std::string propertyName = property.getName();

    propertyList = propertyHolder.getPropertyValue(propertyName);
    for(ItemType item : propertyList)
    {
        jsonValue[componentName][propertyName].append(item);
    }
}

void JetServer::appendMetadataToJsonValue(const ComponentPtr& component)
{
    std::string componentName = toStdString(component.getName());
    std::string globalId = toStdString(component.getGlobalId());
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();
    jsonValue[componentName][typeString] = objectType;
    jsonValue[componentName][globalIdString] = globalId;
}

bool JetServer::determineSelectionProperty(const PropertyPtr& property)
{
    return property.getSelectionValues().assigned() ? true : false;
}

void JetServer::createCallbackForProperty(const PropertyPtr& property)
{
    property.getOnPropertyValueWrite() += [&](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) {
        updateJetState(obj);
    };
}

void JetServer::convertJsonToDaqArguments(BaseObjectPtr& daqArg, const Json::Value& args, const uint16_t& index)
{
    Json::ValueType jsonValueType = args[index].type();
    switch(jsonValueType)
    {
        case Json::ValueType::nullValue:
            std::cout << "Null argument type detected" << std::endl;
            break;
        case Json::ValueType::intValue:
            daqArg.asPtr<IList>().pushBack(args[index].asInt());
            break;
        case Json::ValueType::uintValue:
            daqArg.asPtr<IList>().pushBack(args[index].asUInt());
            break;
        case Json::ValueType::realValue:
            daqArg.asPtr<IList>().pushBack(args[index].asDouble());
            break;
        case Json::ValueType::stringValue:
            daqArg.asPtr<IList>().pushBack(args[index].asString());
            break;
        case Json::ValueType::booleanValue:
            daqArg.asPtr<IList>().pushBack(args[index].asBool());
            break;
        default:
            std::cout << "Unsupported argument detected: " << jsonValueType << std::endl;
    }
}

// TODO! arguments are not received from jet, need to find out why and fix
void JetServer::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
{
    std::string path = jetStatePath + propertyPublisher.getGlobalId() + "/" + property.getName();

    std::string methodName = property.getName();
    CoreType coreType = property.getValueType();

    auto cb = [propertyPublisher, methodName, coreType, this](const Json::Value& args) -> Json::Value
    {
        try
        {
            int numberOfArgs = args.size();
            const BaseObjectPtr method = propertyPublisher.getPropertyValue(methodName);
            if(numberOfArgs > 0)
            {
                BaseObjectPtr daqArg;
                if(numberOfArgs > 1)
                {
                    daqArg = List<IBaseObject>();
                    for (uint16_t i = 0; i < numberOfArgs; ++i)
                    {   
                        convertJsonToDaqArguments(daqArg, args, i);
                    }
                }
                else
                {
                    convertJsonToDaqArguments(daqArg, args, 0);
                }
                if (coreType == ctFunc)
                    method.asPtr<IFunction>()(daqArg);
                else
                    method.asPtr<IProcedure>()(daqArg);

                return "Method called successfully";
            }
            if (coreType == ctFunc)
                method.asPtr<IFunction>()();
            else
                method.asPtr<IProcedure>()();
            
            return "Method called successfully";

        }
        catch(...)
        {
            return "Method called with failure";
        }
        
    };
    jetPeer->addMethodAsync(path, hbk::jet::responseCallback_t(), cb);
}

bool JetServer::checkTypeCompatibility(Json::ValueType jsonValueType, daq::CoreType daqValueType)
{
    switch(jsonValueType)
    {
        case Json::ValueType::intValue:
            {
                if(daqValueType == CoreType::ctInt)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::uintValue:
            {
                if(daqValueType == CoreType::ctInt)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::realValue:
            {
                if(daqValueType == CoreType::ctFloat)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::stringValue:
            {
                if(daqValueType == CoreType::ctString)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::booleanValue:
            {
                if(daqValueType == CoreType::ctBool)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::arrayValue:
            {
                if(daqValueType == CoreType::ctList)
                    return true;
                else
                    return false;
            }
            break;
        default:
            return false;
            break;
    }
}

void JetServer::throwJetModuleException(JetModuleException jmException)
{
    switch(jmException)
    {
        case JetModuleException::JM_INCOMPATIBLE_TYPES:
            {
                std::string message = "Incorrect type detected for openDAQ property";
                std::cout << "addJetState cb: " << message << std::endl;
                throw new hbk::jet::jsoncpprpcException(
                    JM_INCOMPATIBLE_TYPES,                  // code
                    message                                 // message
                    // Json::Value()                        // data
                );
            }
            break;
        case JetModuleException::JM_UNSUPPORTED_JSON_TYPE:
            break;
        case JetModuleException::JM_UNSUPPORTED_DAQ_TYPE:
            break;
    }
}

void JetServer::throwJetModuleException(JetModuleException jmException, Json::ValueType jsonValueType, std::string propertyName, std::string globalId)
{
    switch(jmException)
    {
        case JetModuleException::JM_INCOMPATIBLE_TYPES:
            break;
        case JetModuleException::JM_UNSUPPORTED_JSON_TYPE:
            {
                std::string message = "Update failed for " + propertyName + ", type: " + std::to_string(static_cast<int>(jsonValueType)) + " in " + globalId;
                std::cout << "addJetState cb: " << message << std::endl;
                throw new hbk::jet::jsoncpprpcException(
                    JM_UNSUPPORTED_JSON_TYPE,                   // code
                    message                                     // message
                    // Json::Value()                            // data
                );
            }
            break;
        case JetModuleException::JM_UNSUPPORTED_DAQ_TYPE:
            break;
    }
}

void JetServer::startJetEventloop()
{
    if(!jetEventloopRunning) {
        jetEventloopRunning = true;
        jetEventloop.execute();
    }
}

void JetServer::stopJetEventloop()
{
    if(jetEventloopRunning) {
        jetEventloopRunning = false;
        jetEventloop.stop();
        jetEventloopThread.join();
    }
}

void JetServer::startJetEventloopThread()
{
    jetEventloopThread = std::thread{ &JetServer::startJetEventloop, this };
}

END_NAMESPACE_JET_MODULE
