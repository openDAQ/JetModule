#include "jet_server.h"
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
}

JetServer::~JetServer()
{
    stopJetEventloop();
    delete(jetPeer);
}

void JetServer::addJetState(const std::string& path)
{
    auto cb = [&](const Json::Value& value, std::string path) {
        std::cout << "Want to change state with path: " << path << " with the value " << value.toStyledString() << std::endl;
        return value;
    };
    jetPeer->addStateAsync(path, jsonValue, hbk::jet::responseCallback_t(), cb);
    jsonValue.clear();
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

template <typename PropertyHolderObject>
void JetServer::createJsonProperty(const ComponentPtr& propertyPublisher, const PropertyPtr& property, const PropertyHolderObject& propertyHolderObject)
{
    bool isSelectionProperty = determineSelectionProperty(property);
    std::string propertyName = property.getName();
    if(isSelectionProperty) {
        // TODO
        // Every selection property has associated ctInt associated coretype, so to display actual values, std::string is used.
        // This could be changed.
        std::string propertyValue = propertyHolderObject.getPropertySelectionValue(toStdString(property.getName().toString()));
        appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, propertyValue);
    }
    else {
        bool propertyValueBool;
        int64_t propertyValueInt;
        double propertyValueFloat;
        StringPtr propertyValueString;
        
        CoreType propertyType = property.getValueType();
        switch(propertyType) {
            case CoreType::ctBool:
                propertyValueBool = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<bool>(propertyPublisher, propertyName, propertyValueBool);
                break;
            case CoreType::ctInt:
                propertyValueInt = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<int64_t>(propertyPublisher, propertyName, propertyValueInt);
                break;
            case CoreType::ctFloat:
                propertyValueFloat = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<double>(propertyPublisher, propertyName, propertyValueFloat);
                break;
            case CoreType::ctString:
                propertyValueString = propertyHolderObject.getPropertyValue(propertyName);
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
                auto propertyValue = propertyHolderObject.getPropertyValue(propertyName);
                appendPropertyToJsonValue<std::string>(propertyPublisher, propertyName, propertyValue);
                break;
        }
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
    Json::ValueType valueType = args[index].type();
    switch(valueType)
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
                                std::cout << "Unsupported argument detected: " << valueType << std::endl;
                        }
}

void JetServer::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
{
    std::string path = jetStatePath + propertyPublisher.getGlobalId() + "/" + property.getName();

    std::string methodName = property.getName();
    CoreType coreType = property.getValueType();

    auto cb = [propertyPublisher, methodName, coreType, this]( const Json::Value& args)
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

                return "Method called successfully\n";
            }
            if (coreType == ctFunc)
                method.asPtr<IFunction>()();
            else
                method.asPtr<IProcedure>()();
            
            return "Method called successfully\n";

        }
        catch(...)
        {
            return "Method called with failure\n";
        }
        
    };
    
    jetPeer->addMethodAsync(path, hbk::jet::responseCallback_t(), cb);
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