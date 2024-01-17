#include <iostream>
#include <string>
#include <jet/defines.h>
#include "jet_server.h"
#include "jet_module_exceptions.h"

BEGIN_NAMESPACE_JET_MODULE

JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    propertyCallbacksCreated = false;
    jetStateUpdateDisabled = false;
    jetEventloopRunning = false;

    // initiate openDAQ logger
    logger = LoggerComponent("JetModuleLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);

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
        std::string message = "Want to change state with path: " + path + " with the value " + value.toStyledString() + "\n";
        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
        std::string globalId = removeSubstring(path, jetStatePath);
        ComponentPtr component = componentIdDict.get(globalId);

        // We want to get one "jet state changed" event, so we have to disable state updates until we are finished with updates in opendaq
        jetStateUpdateDisabled = true; 
        // We would like to have "jet state changed" event even if we change at least one property value, at the time when something fails
        bool atLeastOnePropertyChanged = false;

        auto properties = component.getAllProperties();
        for(auto property : properties)
        {
            std::string propertyName = property.getName();
            if (!value.isMember(propertyName))
            {
                std::string message = "Skipping property " + propertyName + "\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                continue;
            }

            Json::ValueType jsonValueType = value.get(propertyName, "").type();
            bool typesAreCompatible = checkTypeCompatibility(jsonValueType, property.getValueType());
            if(!typesAreCompatible)
            {
                throwJetModuleException(JetModuleException::JM_INCOMPATIBLE_TYPES, propertyName);
                continue;
            }

            std::string message = "Changing value for property " + propertyName + "\n";
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
            try
            {
                switch(jsonValueType)
                {
                    case Json::ValueType::intValue:
                        {
                            int64_t oldValue = component.getPropertyValue(propertyName);
                            int64_t newValue = value.get(propertyName, "").asInt64(); 
                            if (oldValue != newValue)
                            {
                                component.setPropertyValue(propertyName, newValue);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::uintValue:
                        {
                            uint64_t oldValue = component.getPropertyValue(propertyName);
                            uint64_t newValue = value.get(propertyName, "").asUInt64();
                            if (oldValue != newValue)
                            {
                                component.setPropertyValue(propertyName, newValue);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::realValue:
                        {
                            double oldValue = component.getPropertyValue(propertyName);
                            double newValue = value.get(propertyName, "").asDouble();
                            if (oldValue != newValue)
                            {
                                component.setPropertyValue(propertyName, newValue);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::stringValue:
                        {
                            std::string oldValue = component.getPropertyValue(propertyName);
                            std::string newValue = value.get(propertyName, "").asString();
                            if (oldValue != newValue)
                            {
                                component.setPropertyValue(propertyName, newValue);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::booleanValue:
                        {
                            bool oldValue = component.getPropertyValue(propertyName);
                            bool newValue = value.get(propertyName, "").asBool();
                            if (oldValue != newValue)
                            {
                                component.setPropertyValue(propertyName, newValue);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::arrayValue:
                        {
                            ListPtr<BaseObjectPtr> oldDaqArray = component.getPropertyValue(propertyName);
                            ListPtr<BaseObjectPtr> newDaqArray = convertJsonArrayToDaqArray(component, propertyName, value);
                            if(oldDaqArray != newDaqArray)
                            {
                                component.setPropertyValue(propertyName, newDaqArray);
                                atLeastOnePropertyChanged = true;
                            }
                            else
                            {
                                std::string message = "Value for " + propertyName + " has not changed. Skipping...\n";
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                        }
                        break;
                    case Json::ValueType::objectValue:
                        {
                            Json::Value obj = value.get(propertyName, Json::Value());
                            convertJsonObjectToDaqObject(component, obj, propertyName + ".");
                        }
                        break;
                    default:
                        if(atLeastOnePropertyChanged == true)
                            updateJetState(component);
                        throwJetModuleException(JetModuleException::JM_UNSUPPORTED_JSON_TYPE, jsonValueType, propertyName, globalId);
                        break;
                }
            }
            catch(...)
            {
                throwJetModuleException(JetModuleException::JM_UNSUPPORTED_JSON_TYPE, jsonValueType, propertyName, globalId);
            }
        }

        jetStateUpdateDisabled = false;
        updateJetState(component);
        return Json::Value();
    };

    jetPeer->addStateAsync(path, jsonValue, hbk::jet::responseCallback_t(), cb);
    jsonValue.clear();
}

void JetServer::updateJetState(const PropertyObjectPtr& propertyObject)
{
    ComponentPtr component = propertyObject.asPtr<IComponent>();
    createJsonProperties(component);   
    appendMetadataToJsonValue(component, jsonValue);

    std::string path = jetStatePath + component.getGlobalId();
    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::updateJetState(const ComponentPtr& component)
{
    createJsonProperties(component);   
    appendMetadataToJsonValue(component, jsonValue);

    std::string path = jetStatePath + component.getGlobalId();
    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::publishJetStates()
{
    createComponentJetState(rootDevice);
    parseFolder(rootDevice);

    propertyCallbacksCreated = true;
}

void JetServer::parseFolder(const FolderPtr& parentFolder)
{
    auto items = parentFolder.getItems();
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto component = item.asPtrOrNull<IComponent>();

       if (channel.assigned())
        {
            createComponentJetState(channel);
        }
        else if (folder.assigned()) // It is important to test for folder last as a channel also is a folder!
        {
            parseFolder(folder); // Folders are recursively parsed until non-folder items are identified in them
        }
        else if (component.assigned())  // It is important to test for component after folder!
        {
            createComponentJetState(component);
        }
        else
        {
            throwJetModuleException(JM_UNSUPPORTED_ITEM);
        }
    }
}

void JetServer::createComponentJetState(const ComponentPtr& component)
{
    componentIdDict.set(component.getGlobalId(), component);
    createJsonProperties(component);   
    appendMetadataToJsonValue(component, jsonValue);
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
void JetServer::createJsonProperty(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    CoreType propertyType = property.getValueType();

    switch(propertyType) {
        case CoreType::ctBool:
            {
                bool propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
        case CoreType::ctInt:
            {
                int64_t propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
        case CoreType::ctFloat:
            {
                double propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
        case CoreType::ctList:
            {
                CoreType listItemType = property.getItemType();
                switch(listItemType)
                {
                    case CoreType::ctBool:
                        appendListPropertyToJsonValue<bool>(propertyHolder, property, parentJsonValue[propertyName]);
                        break;
                    case CoreType::ctInt:
                        appendListPropertyToJsonValue<int>(propertyHolder, property, parentJsonValue[propertyName]);
                        break;
                    case CoreType::ctFloat:
                        appendListPropertyToJsonValue<float>(propertyHolder, property, parentJsonValue[propertyName]);
                        break;
                    case CoreType::ctString:
                        appendListPropertyToJsonValue<std::string>(propertyHolder, property, parentJsonValue[propertyName]);
                        break;
                    default:
                        {
                            std::string message = "Unsupported list item type: " + listItemType + '\n';
                            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
                        }
                }
            }
            break;
        case CoreType::ctString:
            {
                std::string propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
        case CoreType::ctProc:
            createJetMethod(propertyHolder, property);
            break;
        case CoreType::ctFunc:
            createJetMethod(propertyHolder, property);
            break;
        case CoreType::ctStruct:
            {
                StructPtr propertyStruct = propertyHolder.getPropertyValue(propertyName);
                ListPtr<IString> fieldNames = propertyStruct.getFieldNames();
                ListPtr<IBaseObject> fieldValues = propertyStruct.getFieldValues();

                for(int i = 0; i < fieldNames.getCount(); i++)
                {
                    CoreType structfieldType = fieldValues[i].getCoreType();
                    switch(structfieldType)
                    {
                        case CoreType::ctBool:
                            {
                                bool fieldValue = fieldValues[i];
                                parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                            }
                            break;
                        case CoreType::ctInt:
                            {
                                int64_t fieldValue = fieldValues[i];
                                parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                            }
                            break;
                        case CoreType::ctFloat:
                            {
                                double fieldValue = fieldValues[i];
                                parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                            }
                            break;
                        case CoreType::ctString:
                            {
                                std::string fieldValue = fieldValues[i];
                                parentJsonValue[propertyName][toStdString(fieldNames[i])] = fieldValue;
                            }
                            break;
                        case CoreType::ctList:
                            {
                                ListPtr<IBaseObject> fieldValue = fieldValues[i];
                                CoreType listItemType = fieldValue.asPtr<IProperty>().getItemType();
                                switch(listItemType)
                                {
                                    case CoreType::ctBool:
                                        appendListPropertyToJsonValue<bool>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                                        break;
                                    case CoreType::ctInt:
                                        appendListPropertyToJsonValue<int>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                                        break;
                                    case CoreType::ctFloat:
                                        appendListPropertyToJsonValue<float>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                                        break;
                                    case CoreType::ctString:
                                        appendListPropertyToJsonValue<std::string>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                                        break;
                                    default:
                                        {
                                            std::string message = "Unsupported list item type: " + listItemType + '\n';
                                            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
                                        }
                                    break;
                                }
                            }
                        case CoreType::ctDict:
                            break;
                        case CoreType::ctRatio:
                            break;
                        case CoreType::ctComplexNumber:
                            break;
                        default:
                            {
                                std::string message = "Unsupported struct field type: " + structfieldType + '\n';
                                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                            }
                    }
                }

                std::string structValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = structValue;
            }
            break;
        case CoreType::ctObject:
            {
                PropertyObjectPtr propertyObject = propertyHolder.getPropertyValue(propertyName);
                std::string propertyObjectName = property.getName();

                auto properties = propertyObject.getAllProperties();
                for(auto property : properties)
                {
                    createJsonProperty<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
                }
            }
            break;
        default:
            {
                std::string message = "Unsupported value type of Property: " + propertyName + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
                message = "\"std::string\" will be used to store property value.\n";
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
                std::string propertyValue = propertyHolder.getPropertyValue(propertyName);
                parentJsonValue[propertyName] = propertyValue;
            }
            break;
    }
}

void JetServer::createJsonProperties(const ComponentPtr& component)
{
    std::string propertyPublisherName = component.getName();

    auto properties = component.getAllProperties();
    for(auto property : properties) {
        createJsonProperty<ComponentPtr>(component, property, jsonValue);
        if(!propertyCallbacksCreated)
            createCallbackForProperty(property);
    }
}

template <typename ValueType>
void JetServer::appendPropertyToJsonValue(const ComponentPtr& component, const std::string& propertyName, const ValueType& value)
{
    std::string componentName = toStdString(component.getName());
    jsonValue[componentName][propertyName] = value;
}

template <typename ItemType>
void JetServer::appendListPropertyToJsonValue(const ComponentPtr& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    ListPtr<ItemType> propertyList = List<ItemType>();
    propertyList = propertyHolder.getPropertyValue(propertyName);

    for(ItemType item : propertyList)
    {
        parentJsonValue.append(item);
    }
}

void JetServer::appendMetadataToJsonValue(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string globalId = toStdString(component.getGlobalId());
        parentJsonValue["Global ID"] = globalId;
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();
        parentJsonValue["_type"] = objectType;
    bool isActive = component.getActive();
        parentJsonValue["Active"] = isActive;
    TagsConfigPtr tags = component.getTags();
        auto tagsList = tags.getList();
        for(const std::string& item : tagsList)
            parentJsonValue["Tags"].append(item);

    if(strcmp(component.asPtr<ISerializable>().getSerializeId(), "Device") == 0)
    {   
        // Device Info
        // Checking whether the component is a device. If it's a device we have to get deviceInfo properties manually
        auto deviceInfo = component.asPtr<IDevice>().getInfo();
        auto deviceInfoProperties = deviceInfo.getAllProperties();
        for(auto property : deviceInfoProperties) 
        {
            createJsonProperty<DeviceInfoPtr>(deviceInfo, property, jsonValue);
            if(!propertyCallbacksCreated)
                createCallbackForProperty(property);
        }
        
        // Device Domain
        DeviceDomainPtr domain = component.asPtr<IDevice>().getDomain();
        RatioPtr tickResolution = domain.getTickResolution();
            int64_t numerator = tickResolution.getNumerator();
            int64_t denominator = tickResolution.getDenominator();
            parentJsonValue["Domain"]["Resolution"]["Numerator"] = numerator;
            parentJsonValue["Domain"]["Resolution"]["Denominator"] = denominator;
        uint64_t ticksSinceResolution = domain.getTicksSinceOrigin();
            parentJsonValue["Domain"]["TicksSinceOrigin"] = ticksSinceResolution;
        std::string origin = domain.getOrigin();
            parentJsonValue["Domain"]["Origin"] = origin;
        UnitPtr unit = domain.getUnit();
            int64_t id = unit.getId();
            std::string name = unit.getName();
            std::string quantity = unit.getQuantity();
            std::string symbol = unit.getSymbol();
            parentJsonValue["Domain"]["Unit"]["UnitId"] = id;
            parentJsonValue["Domain"]["Unit"]["Description"] = name;
            parentJsonValue["Domain"]["Unit"]["Quantity"] = quantity;
            parentJsonValue["Domain"]["Unit"]["DisplayName"] = symbol;
    }
}

void JetServer::createCallbackForProperty(const PropertyPtr& property)
{
    property.getOnPropertyValueWrite() += [&](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) {
        if(!jetStateUpdateDisabled)
            updateJetState(obj);
    };
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

StringPtr JetServer::getJetStatePath()
{
    return jetStatePath;
}
    
void JetServer::setJetStatePath(StringPtr path)
{
    jetStatePath = path;
}


END_NAMESPACE_JET_MODULE
