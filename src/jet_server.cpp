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

    std::string path = jetStatePath + component.getGlobalId();
    jetPeer->notifyState(path, jsonValue);
    jsonValue.clear();
}

void JetServer::updateJetState(const ComponentPtr& component)
{
    createJsonProperties(component);   

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

    // Adding additional information to a component's Jet state
    appendGlobalId(component, jsonValue);
    appendObjectType(component, jsonValue);
    appendActiveStatus(component, jsonValue);
    appendTags(component, jsonValue);    

    const char* componentType = component.asPtr<ISerializable>().getSerializeId(); // the value will be e.g. "Device", "Channel" and so on
    // Checking whether the component is a device. If it's a device we have to retrieve device metadata properties
    if(strcmp(componentType, "Device") == 0)
    {   
        appendDeviceMetadata(component.asPtr<IDevice>(), jsonValue);
        appendDeviceDomain(component.asPtr<IDevice>(), jsonValue);
        appendOutputSignals<DevicePtr>(component, jsonValue);
    }
    else if(strcmp(componentType, "Channel") == 0) {
        appendFunctionBlockInfo(component.asPtr<IFunctionBlock>(), jsonValue);
        appendInputPorts(component.asPtr<IFunctionBlock>(), jsonValue);
        appendOutputSignals<ChannelPtr>(component, jsonValue);
    }

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
            appendSimpleProperty<PropertyHolder, bool>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctInt:
            appendSimpleProperty<PropertyHolder, int64_t>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctFloat:
            appendSimpleProperty<PropertyHolder, double>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctString:
            appendSimpleProperty<PropertyHolder, std::string>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctList:
            appendListProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctDict:
            appendDictProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctRatio:
            appendRatioProperty<PropertyHolder>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctComplexNumber:
            // TODO: Needs verification that it works. Have to create a list of complex numbers
            appendComplexNumber<PropertyHolder>(propertyHolder, propertyName, parentJsonValue);
            break;
        case CoreType::ctStruct:
            appendStructProperty<PropertyHolder>(propertyHolder, property, parentJsonValue);
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
        case CoreType::ctProc:
            createJetMethod(propertyHolder, property);
            break;
        case CoreType::ctFunc:
            createJetMethod(propertyHolder, property);
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

/**
 * @brief Appends simple properties types BoolProperty, IntProperty, FloatProperty and StringProperty to Json::Value object,
 * in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @tparam DataType Type of the property - bool, in64_t, double or std::string.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType, typename DataType>
void JetServer::appendSimpleProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue) {
    DataType propertyValue = propertyHolder.getPropertyValue(propertyName);
    parentJsonValue[propertyName] = propertyValue;
}

/**
 * @brief Appends ListProperty to Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendListProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    CoreType listItemType = property.getItemType();
    switch(listItemType)
    {
        case CoreType::ctBool:
            fillListProperty<PropertyHolderType, bool>(propertyHolder, propertyName, parentJsonValue[propertyName]);
            break;
        case CoreType::ctInt:
            fillListProperty<PropertyHolderType, int64_t>(propertyHolder, propertyName, parentJsonValue[propertyName]);
            break;
        case CoreType::ctFloat:
            fillListProperty<PropertyHolderType, double>(propertyHolder, propertyName, parentJsonValue[propertyName]);
            break;
        case CoreType::ctString:
            fillListProperty<PropertyHolderType, std::string>(propertyHolder, propertyName, parentJsonValue[propertyName]);
            break;
        case CoreType::ctRatio:
            fillListPropertyWithRatio<PropertyHolderType>(propertyHolder, propertyName, parentJsonValue[propertyName]);
            break;
        case CoreType::ctComplexNumber:
            // TODO: has to be implemented
            break;
        default:
            {
                std::string message = "Unsupported list item type: " + listItemType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
    }
}

/**
 * @brief Helper function which appends a ListProperty to a Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @tparam ItemType Type of the List items.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template <typename PropertyHolderType, typename ItemType>
void JetServer::fillListProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    ListPtr<ItemType> propertyList = List<ItemType>();
    propertyList = propertyHolder.getPropertyValue(propertyName);

    for(const ItemType& item : propertyList) {
        parentJsonValue.append(item);
    }
}

/**
 * @brief Helper function which appends Ratio type object to a ListProperty in a a Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template <typename PropertyHolderType>
void JetServer::fillListPropertyWithRatio(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    ListPtr<IRatio> ratioList = List<IRatio>();
    ratioList = propertyHolder.getPropertyValue(propertyName);

    for(const RatioPtr& ratio : ratioList) {
        Json::Value ratioJson;
        int64_t numerator = ratio.getNumerator();
        int64_t denominator = ratio.getDenominator();
        ratioJson["Numerator"] = numerator;
        ratioJson["Denominator"] = denominator;
        parentJsonValue.append(ratioJson);
    }
}

// non-string key types cannot be represented in Json::Value!!

/**
 * @brief Appends DictProperty to Json::Value object in order to be represented in a Jet state. DictProperty is a collection of key-value pairs.
 * Non-string key types cannot be represented in Json::Value.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    CoreType keyCoreType = property.getKeyType();
    //! Non-string key types cannot be represented in Json::Value!
    switch(keyCoreType) {
        // case CoreType::ctBool:
            // determineDictItemType<PropertyHolderType, bool>(propertyHolder, property, parentJsonValue);
            // break;
        // case CoreType::ctInt:
            // determineDictItemType<PropertyHolderType, int64_t>(propertyHolder, property, parentJsonValue);
            // break;
        // case CoreType::ctFloat:
            // determineDictItemType<PropertyHolderType, double>(propertyHolder, property, parentJsonValue);
            // break;
        case CoreType::ctString:
            determineDictItemType<PropertyHolderType, std::string>(propertyHolder, property, parentJsonValue);
            break;
        default:
            {
                std::string message = "Unsupported dictionary key type: " + keyCoreType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
    }
}

/**
 * @brief Helper function which determines item type (value type key-value pairs) of a DictProperty.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @tparam KeyType Type of the keys (types of keys in key-value pairs) in a DictProperty.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template <typename PropertyHolderType, typename KeyType>
void JetServer::determineDictItemType(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    CoreType itemCoreType = property.getItemType();

    switch(itemCoreType) {
        case CoreType::ctBool:
            fillDictProperty<PropertyHolderType, KeyType, bool>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctInt:
            fillDictProperty<PropertyHolderType, KeyType, int64_t>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctFloat:
            fillDictProperty<PropertyHolderType, KeyType, double>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctString:
            fillDictProperty<PropertyHolderType, KeyType, std::string>(propertyHolder, property, parentJsonValue);
            break;
        case CoreType::ctRatio:
            // TODO: has to be implemented
            break;
        case CoreType::ctComplexNumber:
            // TODO: has to be implemented
            break;
        default:
            {
                std::string message = "Unsupported dictionary item type: " + itemCoreType + '\n';
                logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Error);
            }
            break;
    }
}

/**
 * @brief Helper function which appends a DictProperty to a Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @tparam KeyType Type of the keys (types of keys in key-value pairs) in a DictProperty.
 * @tparam ItemType Type of the items (values of keys in key-value pairs) in a DictProperty.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template <typename PropertyHolderType, typename KeyType, typename ItemType>
void JetServer::fillDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    DictPtr<KeyType, ItemType> dict = propertyHolder.getPropertyValue(propertyName);
    // ListPtr<KeyType> keyList = dict.getKeyList();
    ListPtr<std::string> keyList = dict.getKeyList(); // Dictionaries with only std::string key types can be represented in Json format!
    ListPtr<ItemType> itemList = dict.getValueList();

    for(size_t i = 0; i < dict.getCount(); i++) {
        std::string key = keyList[i];
        ItemType value = itemList[i];
        parentJsonValue[propertyName][key] = value;
    }
}

/**
 * @brief Appends RatioProperty to Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendRatioProperty(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    RatioPtr ratioProperty = propertyHolder.getPropertyValue(propertyName);
    int64_t numerator = ratioProperty.getNumerator();
    int64_t denominator = ratioProperty.getDenominator();
    parentJsonValue[propertyName]["Numerator"] = numerator;
    parentJsonValue[propertyName]["Denominator"] = denominator;
}

//! This function needs testing!
/**
 * @brief Appends ComplexNumber object to Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendComplexNumber(const PropertyHolderType& propertyHolder, const std::string& propertyName, Json::Value& parentJsonValue)
{
    ComplexNumberPtr complexNumber = propertyHolder.getPropertyValue(propertyName);
    double real = complexNumber.getReal();
    double imag = complexNumber.getImaginary();
    parentJsonValue[propertyName]["Real"] = real;
    parentJsonValue[propertyName]["Imaginary"] = imag;
}

//! This function is not finished!
/**
 * @brief Appends StructProperty to Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendStructProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    StructPtr propertyStruct = propertyHolder.getPropertyValue(propertyName);
    ListPtr<IString> fieldNames = propertyStruct.getFieldNames();
    ListPtr<IBaseObject> fieldValues = propertyStruct.getFieldValues();

    for(size_t i = 0; i < fieldNames.getCount(); i++)
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
                            {
                                ListPtr<bool> propertyList = fieldValues[i];

                                for(const bool& item : propertyList) {
                                    parentJsonValue[propertyName][toStdString(fieldNames[i])].append(item);
                                }
                            }
                            break;
                        case CoreType::ctInt:
                            // appendListPropertyToJsonValue<int>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                            break;
                        case CoreType::ctFloat:
                            // appendListPropertyToJsonValue<float>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
                            break;
                        case CoreType::ctString:
                            // appendListPropertyToJsonValue<std::string>(propertyHolder, property, parentJsonValue[propertyName][toStdString(fieldNames[i])]);
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

/**
 * @brief Appends ObjectProperty to Json::Value object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json::Value object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendObjectProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    PropertyObjectPtr propertyObject = propertyHolder.getPropertyValue(propertyName);
    std::string propertyObjectName = property.getName();

    auto properties = propertyObject.getAllProperties();
    for(auto property : properties)
    {
        createJsonProperty<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
    }
}

/**
 * @brief Appends device metadata information to Json::Value object which is published as a Jet state. 
 * 
 * @param device Device from which metadata is retrieved.
 * @param parentJsonValue Json::Value object to which metadata is appended.
 */
void JetServer::appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue)
{
    auto deviceInfo = device.getInfo();
    auto deviceInfoProperties = deviceInfo.getAllProperties();
    for(auto property : deviceInfoProperties) 
    {
        createJsonProperty<DeviceInfoPtr>(deviceInfo, property, parentJsonValue);
        if(!propertyCallbacksCreated)
            createCallbackForProperty(property); // TODO: callbacks for device information properties don't seem to work. Need to find a way around
    }
}

/**
 * @brief Appends device domain data (e.g. time domain information) to Json::Value object which is published as a Jet state. 
 * 
 * @param device Device from which domain data is retrieved.
 * @param parentJsonValue Json::Value object to which domain data is appended.
 */
void JetServer::appendDeviceDomain(const DevicePtr& device, Json::Value& parentJsonValue)
{
    DeviceDomainPtr domain = device.getDomain();
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

/**
 * @brief Appends FunctionBlockInfo to Json::Value object which is published as a Jet state. 
 * FunctionBlockInfo is an information structure which contains metadata of the function block type.
 * 
 * @param functionBlock Function block from which FunctionBlockInfo structure is retrieved. Channel is a function block as well.
 * @param parentJsonValue  Json::Value object to which FunctionBlockInfo structure is appended.
 */
void JetServer::appendFunctionBlockInfo(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue)
{
    const FunctionBlockTypePtr fbType = functionBlock.getFunctionBlockType();
    std::string fbId = fbType.getId();
    std::string fbName = fbType.getName();
    std::string fbDescription = fbType.getDescription();
    parentJsonValue["FunctionBlockInfo"]["Id"] = fbId;
    parentJsonValue["FunctionBlockInfo"]["Name"] = fbName;
    parentJsonValue["FunctionBlockInfo"]["Description"] = fbDescription;

}

/**
 * @brief Appends input ports to Json::Value object which is published as a Jet state.
 * 
 * @param functionBlock Function block from which input ports are retrieved. Channel is a function block as well.
 * @param parentJsonValue Json::Value object to which input ports are appended.
 */
void JetServer::appendInputPorts(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue)
{
    const auto inputPorts = functionBlock.getInputPorts();
    for(auto inputPort : inputPorts) {
        std::string name = inputPort.getName();

        // Adding metadata
        appendGlobalId(inputPort, parentJsonValue["InputPorts"][name]);
        appendObjectType(inputPort, parentJsonValue["InputPorts"][name]);
        appendActiveStatus(inputPort, parentJsonValue["InputPorts"][name]);
        appendTags(inputPort, parentJsonValue["InputPorts"][name]);

        bool requiresSignal = inputPort.getRequiresSignal();
        parentJsonValue["InputPorts"][name]["RequiresSignal"] = requiresSignal;
    }
    
    if(inputPorts.getCount() == 0) // If there are no input ports, still add "InputPorts" attribute to Json object with null value
        parentJsonValue["InputPorts"] = Json::ValueType::nullValue;
}

/**
 * @brief Appends output signals to Json::Value object which is published as a Jet state.
 * 
 * @tparam ObjectType Type of object from which signals are retrieved (e.g. DevicePtr, ChannelPtr...)
 * @param object Object from which signals are retrieved.
 * @param parentJsonValue Json::Value object to which signals are appended.
 */
template <typename ObjectType>
void JetServer::appendOutputSignals(const ObjectType& object, Json::Value& parentJsonValue)
{
    const auto signals = object.getSignals();
    for(auto signal : signals) {
        std::string signalName = signal.getName();

        // Adding metadata
        appendGlobalId(signal, parentJsonValue["OutputSignals"][signalName]);
        appendObjectType(signal, parentJsonValue["OutputSignals"][signalName]);
        appendActiveStatus(signal, parentJsonValue["OutputSignals"][signalName]);
        appendTags(signal, parentJsonValue["OutputSignals"][signalName]);

        DataDescriptorPtr dataDescriptor = signal.getDescriptor();
        std::string name = dataDescriptor.getName();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Name"] = name;
        ListPtr<IDimension> dimensions = dataDescriptor.getDimensions();
            size_t dimensionsCount = dimensions.getCount();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Dimensions"] = dimensionsCount;
        DictPtr<IString, IString> metadata = dataDescriptor.getMetadata();
            size_t metadataCount = metadata.getCount();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Metadata"] = metadataCount;
        DataRulePtr rule = dataDescriptor.getRule();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Rule"] = std::string(rule);
        SampleType sampleType;
            // SampleType::Invalid results in runtime error when we try to get the sample type
            // Due to this, getter function is inside of try-catch block
            try {
                sampleType = dataDescriptor.getSampleType();
            } catch(...) {
                sampleType = SampleType::Invalid;
            }
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["SampleType"] = int(sampleType);
        UnitPtr unit = dataDescriptor.getUnit();
            int64_t unitId = unit.getId();
            std::string unitName = unit.getName();
            std::string unitQuantity = unit.getQuantity();
            std::string unitSymbol = unit.getSymbol();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Unit"]["UnitId"] = unitId;
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Unit"]["Description"] = unitName;
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Unit"]["Quantity"] = unitQuantity;
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Unit"]["DisplayName"] = unitSymbol;
        ScalingPtr postScaling = dataDescriptor.getPostScaling();
            // SampleType::Invalid and ScaledSampleType::Invalid result in runtime error when we try to get them
            // Due to thism getter functions are inside of try-catch blocks
            SampleType postScalingInputSampleType;
            ScaledSampleType postScalingOutputSampleType;
            try {
                postScalingInputSampleType = postScaling.getInputSampleType();
            } catch(...) {
                postScalingInputSampleType = SampleType::Invalid;
            }
            try {
                postScalingOutputSampleType = postScaling.getOutputSampleType();
            } catch(...) {
                postScalingOutputSampleType = ScaledSampleType::Invalid;
            }
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["PostScaling"]["InputSampleType"] = int(postScalingInputSampleType);
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["PostScaling"]["OutputSampleType"] = int(postScalingOutputSampleType);
        StringPtr origin = dataDescriptor.getOrigin();
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["Origin"] = toStdString(origin);
        RatioPtr tickResolution = dataDescriptor.getTickResolution();
            // If the tick resolution is not applicable to the signal, getter functions throw runtime errors
            // Due to this, we have to use try-catch block and assign 0s as default values in that case
            int64_t numerator;
            int64_t denominator;
            try {
                numerator = tickResolution.getNumerator();
                denominator = tickResolution.getDenominator();
            } catch(...) {
                numerator = 0;
                denominator = 0;
            }
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["TickResolution"]["Numerator"] = numerator;
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["TickResolution"]["Denominator"] = denominator;
        RangePtr valueRange = dataDescriptor.getValueRange();
            // If the value range is not applicable to the signal, getter functions throw runtime errors
            // Due to this, we have to use try-catch block and assign 0s as default values in that case
            double lowValue;
            double highValue;
            try {
                lowValue = valueRange.getLowValue();
                highValue = valueRange.getHighValue();
            } catch(...) {
                lowValue = 0;
                highValue = 0;
            }
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["ValueRange"]["Low"] = lowValue;
            parentJsonValue["OutputSignals"][signalName]["Value"]["DataDescriptor"]["ValueRange"]["High"] = highValue;
    }

    if(signals.getCount() == 0) // If there are no signals, still add "OutputSignals" attribute to Json object with null value
        parentJsonValue["OutputSignals"] = Json::ValueType::nullValue;
}

/**
 * @brief Appends Global ID (global unique identified of the object) to Json::Value object which is published as a Jet state.
 * 
 * @param component Component from which Global ID is retrieved.
 * @param parentJsonValue Json::Value object to which Global ID is appended.
 */
void JetServer::appendGlobalId(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string globalId = component.getGlobalId();
    parentJsonValue["Global ID"] = globalId;
}

/**
 * @brief Appends type of the object (e.g. Device, Channel...) to Json::Value object which is published as a Jet state.
 * 
 * @param component Component from which type is retrieved.
 * @param parentJsonValue Json::Value object to which component's type is appended.
 */
void JetServer::appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();
    parentJsonValue["_type"] = objectType;
}

/**
 * @brief Appends a component's activity status (true or false) to Json::Value object which is published as a Jet state.
 * 
 * @param component Component from which activity status is retrieved.
 * @param parentJsonValue Json::Value object to which activity status is appended.
 */
void JetServer::appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    bool isActive = component.getActive();
    parentJsonValue["Active"] = isActive;
}

/**
 * @brief Appends tags (user definable labels) to Json::Value object which is published as a Jet state.
 * 
 * @param component Component from which tags are retrieved.
 * @param parentJsonValue Json::Value object to which tags are appended.
 */
void JetServer::appendTags(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    TagsPtr tags = component.getTags();
    auto tagsList = tags.getList();
    for(const std::string& item : tagsList)
        parentJsonValue["Tags"].append(item);
    
    if(tagsList.getCount() == 0) // If there are no tags, still add "Tags" attribute to Json object with null value
        parentJsonValue["Tags"] = Json::ValueType::nullValue;
}

END_NAMESPACE_JET_MODULE
