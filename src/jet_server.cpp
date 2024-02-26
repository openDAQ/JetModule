#include <iostream>
#include <string>
#include <jet/defines.h>
#include "jet_server.h"
#include "jet_module_exceptions.h"
#include "opendaq_to_json_converters.h"
#include "json_to_opendaq_converters.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Constructs a new Jet Server object. It takes an openDAQ device as an argument and publishes its tree structure in Json representation
 * as Jet states.
 * 
 * @param device A device which will be parsed and structure of which is published as Jet states.
 */
JetServer::JetServer(const DevicePtr& device)
{
    this->rootDevice = device;
    jetEventloopRunning = false;

    startJetEventloopThread();
    jetPeer = new hbk::jet::PeerAsync(jetEventloop, hbk::jet::JET_UNIX_DOMAIN_SOCKET_NAME, 0);
}

/**
 * @brief Destroy the JetServer object. It stops the Jet eventloop and deletes dynamically created PeerAsync object.
 * 
 */
JetServer::~JetServer()
{
    stopJetEventloop();
    delete(jetPeer);
}

/**
 * @brief Publishes a device's tree structure in Json format as Jet states.
 * 
 */
void JetServer::publishJetStates()
{
    prepareComponentJetState(rootDevice); // Need to parse root device sepaately because parsing in parseFolder function is done relative to it
    parseFolder(rootDevice);
}

/**
 * @brief Parses a openDAQ folder to identify components in it. The components are parsed themselves to create their Jet states.
 * 
 * @param parentFolder A folder which is parsed to identify components in it.
 */
void JetServer::parseFolder(const FolderPtr& parentFolder)
{
    auto items = parentFolder.getItems();
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto component = item.asPtrOrNull<IComponent>();
        auto functionBlock = item.asPtrOrNull<IFunctionBlock>();

        if(channel.assigned()) {
            prepareComponentJetState(channel);
        }
        else if(functionBlock.assigned()) {
            prepareComponentJetState(functionBlock);
        }
        else if(folder.assigned()) { // It is important to test for folder last as a channel also is a folder!
            parseFolder(folder); // Folders are recursively parsed until non-folder items are identified in them
        }
        else if(component.assigned()) { // It is important to test for component after folder!
            prepareComponentJetState(component);
        }
        else {
            throwJetModuleException(JM_UNSUPPORTED_ITEM);
        }
    }
}

/**
 * @brief Prepares a component for publishing its tree structure in Json format as a Jet state.
 * 
 * @param component Component object whose tree structure is published.
 */
void JetServer::prepareComponentJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    parseComponentProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendGlobalId(component, jetState);
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendTags(component, jetState);    

    // Checking the concrete type of the component. Depending on whether it's a device or channel, specific objects have to be
    // appended to its Json representation in a Jet state
    const char* componentType = component.asPtr<ISerializable>().getSerializeId(); // the value is e.g. "Device", "Channel" and so on
    if(strcmp(componentType, "Device") == 0)
    {   
        appendDeviceMetadata(component.asPtr<IDevice>(), jetState);
        appendDeviceDomain(component.asPtr<IDevice>(), jetState);
        parseComponentSignals<DevicePtr>(component, jetState);
    }
    else if(strcmp(componentType, "Channel") == 0) {
        appendFunctionBlockInfo(component.asPtr<IFunctionBlock>(), jetState);
        parseComponentInputPorts(component.asPtr<IFunctionBlock>(), jetState);
        parseComponentSignals<ChannelPtr>(component, jetState);
        appendVisibleStatus(component, jetState);
    }
    else if(strcmp(componentType, "FunctionBlock") == 0) {
        appendFunctionBlockInfo(component.asPtr<IFunctionBlock>(), jetState);
        parseComponentInputPorts(component.asPtr<IFunctionBlock>(), jetState);
        parseComponentSignals<FunctionBlockPtr>(component, jetState);
        appendVisibleStatus(component, jetState);
    }

    createComponentCallback(component); // Creating openDAQ callbacks

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    publishComponentJetState(path, jetState);
}

/**
 * @brief Parses a signal and prepares its Json representation for publishing as a Jet state.
 * 
 * @param signal Pointer to the signal which is parsed.
 */
void JetServer::prepareSignalJetState(const SignalPtr& signal)
{
    Json::Value signalJetState;
    std::string signalName = signal.getName();

    // Adding metadata
    appendGlobalId(signal, signalJetState);
    appendObjectType(signal, signalJetState);
    appendActiveStatus(signal, signalJetState);
    appendVisibleStatus(signal, signalJetState);
    appendTags(signal, signalJetState);

    DataDescriptorPtr dataDescriptor = signal.getDescriptor();

    // If data descriptor is empty add an empty Json enrtry
    if(dataDescriptor.assigned() == false) {
        signalJetState["Value"]["DataDescriptor"] = Json::ValueType::nullValue;
        return;
    }

    std::string name = dataDescriptor.getName();
        signalJetState["Value"]["DataDescriptor"]["Name"] = name;
    ListPtr<IDimension> dimensions = dataDescriptor.getDimensions();
        size_t dimensionsCount = dimensions.getCount();
        signalJetState["Value"]["DataDescriptor"]["Dimensions"] = dimensionsCount;
    DictPtr<IString, IString> metadata = dataDescriptor.getMetadata();
        size_t metadataCount = metadata.getCount();
        signalJetState["Value"]["DataDescriptor"]["Metadata"] = metadataCount;
    DataRulePtr rule = dataDescriptor.getRule();
        signalJetState["Value"]["DataDescriptor"]["Rule"] = std::string(rule);
    SampleType sampleType;
        // SampleType::Invalid results in runtime error when we try to get the sample type
        // Due to this, getter function is inside of try-catch block
        try {
            sampleType = dataDescriptor.getSampleType();
        } catch(...) {
            sampleType = SampleType::Invalid;
        }
        signalJetState["Value"]["DataDescriptor"]["SampleType"] = int(sampleType);
    UnitPtr unit = dataDescriptor.getUnit();
        int64_t unitId = unit.getId();
        std::string unitName = unit.getName();
        std::string unitQuantity = unit.getQuantity();
        std::string unitSymbol = unit.getSymbol();
        signalJetState["Value"]["DataDescriptor"]["Unit"]["UnitId"] = unitId;
        signalJetState["Value"]["DataDescriptor"]["Unit"]["Description"] = unitName;
        signalJetState["Value"]["DataDescriptor"]["Unit"]["Quantity"] = unitQuantity;
        signalJetState["Value"]["DataDescriptor"]["Unit"]["DisplayName"] = unitSymbol;
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
        signalJetState["Value"]["DataDescriptor"]["PostScaling"]["InputSampleType"] = int(postScalingInputSampleType);
        signalJetState["Value"]["DataDescriptor"]["PostScaling"]["OutputSampleType"] = int(postScalingOutputSampleType);
    StringPtr origin = dataDescriptor.getOrigin();
        signalJetState["Value"]["DataDescriptor"]["Origin"] = toStdString(origin);
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
        signalJetState["Value"]["DataDescriptor"]["TickResolution"]["Numerator"] = numerator;
        signalJetState["Value"]["DataDescriptor"]["TickResolution"]["Denominator"] = denominator;
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
        signalJetState["Value"]["DataDescriptor"]["ValueRange"]["Low"] = lowValue;
        signalJetState["Value"]["DataDescriptor"]["ValueRange"]["High"] = highValue;
    
    publishComponentJetState(signal.getGlobalId(), signalJetState);
}

/**
 * @brief Parses an input port and prepares its Json representation for publishing as a Jet state.
 * 
 * @param inputPort Pointer to the input port which is parsed.
 */
void JetServer::prepareInputPortJetState(const InputPortPtr& inputPort)
{
    Json::Value inputPortJetState;
    std::string inputPortName = inputPort.getName();

    // Adding metadata
    appendGlobalId(inputPort, inputPortJetState);
    appendObjectType(inputPort, inputPortJetState);
    appendActiveStatus(inputPort, inputPortJetState);
    appendVisibleStatus(inputPort, inputPortJetState);
    appendTags(inputPort, inputPortJetState);

    bool requiresSignal = inputPort.getRequiresSignal();
    inputPortJetState["RequiresSignal"] = requiresSignal;

    publishComponentJetState(inputPort.getGlobalId(), inputPortJetState);
}

/**
 * @brief Defines a callback function for a component which will be called when some event occurs in a structure
 * of an openDAQ component.
 * 
 * @param component Pointer to the component for which a callback function is defined.
 */
void JetServer::createComponentCallback(const ComponentPtr& component)
{
    component.getOnComponentCoreEvent() += [this](const ComponentPtr& comp, const CoreEventArgsPtr& args)
    {
        auto parameters = args.getParameters();
        auto keyList = parameters.getKeyList();
        auto valueList = parameters.getValueList();

        DictPtr<IString, IBaseObject> eventParameters = args.getParameters();
        if(eventParameters.hasKey("Name")) { // Properties
            OPENDAQ_EVENT_updateProperty(comp, eventParameters);
        }
        else if(eventParameters.hasKey("Active")) { // Active status
            OPENDAQ_EVENT_updateActiveStatus(comp, eventParameters);
        }
    };

}

/**
 * @brief Publishes a Json value as a Jet state to the specified path.
 * 
 * @param path Path which the Jet state will have.
 * @param jetState Json representation of the Jet state.
 */
void JetServer::publishComponentJetState(const std::string& path, const Json::Value& jetState)
{
    auto cb = [this](const Json::Value& value, std::string path) -> Json::Value
    {
        std::string message = "Want to change state with path: " + path + " with the value " + value.toStyledString() + "\n";
        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
        
        // Actual work is done on a separate thread to handle simultaneous requests. Also, otherwise "jetset" tool would time out
        std::thread([this, value, path]() 
        {   
            // We find component by searching relative to root device, so we have to remove its name from global ID of the component with provided path
            std::string relativePath = removeRootDeviceId(path);

            ComponentPtr component = rootDevice.findComponent(relativePath);

            Json::Value jetState = readJetState(path);

            for (auto it = value.begin(); it != value.end(); ++it) {
                std::string entryName = it.key().asString();
                Json::Value entryValue = *it;

                if(component.hasProperty(entryName)) {
                    JET_EVENT_updateProperty(component, entryName, entryValue);
                }
                else if(entryName == "Active") {
                    JET_EVENT_updateActiveStatus(component, entryValue);
                }
                else if(entryName == "Tags") {
                    // TODO: Implement a function which updates tags
                }
            }

        }).detach(); // detach is used to separate the thread of execution from the thread object, allowing execution to continue independently

        return Json::Value(); // Return an empty Json as there's no need to return anything specific.
        // TODO: Make sure that this is ok
    };

    jetPeer->addStateAsync(path, jetState, hbk::jet::responseCallback_t(), cb);
}

/**
 * @brief Parses a component to get its properties which are converted into Json representation in order to be published
 * in the component's Jet state
 * 
 * @param component Component which is parsed to retrieve its properties.
 * @param parentJsonValue Json object which is filled with representations of openDAQ properties.
 */
void JetServer::parseComponentProperties(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string propertyPublisherName = component.getName();

    auto properties = component.getAllProperties();
    for(auto property : properties) {
        determinePropertyType<ComponentPtr>(component, property, parentJsonValue);
    }
}

/**
 * @brief Parses a component to identify its signals and create Json representation for each of them, which then are published
 * as individual Jet states.
 * 
 * @tparam ObjectType Type of the object which contatins signals.
 * @param object Object which contains signals.
 * @param parentJsonValue Json object to which signal names are appended.
 */
template <typename ObjectType>
void JetServer::parseComponentSignals(const ObjectType& object, Json::Value& parentJsonValue)
{
    const auto signals = object.getSignals();
    
    // Add an empty Json entry if the object doesn't have signals
    if(signals.getCount() == 0)
    {
        parentJsonValue["OutputSignals"] = Json::ValueType::nullValue;
        return;
    }

    for(auto signal : signals) {
        std::string signalName = signal.getName();
        parentJsonValue["OutputSignals"].append(signalName); // Add a list entry to the parent object consisting of signal names

        prepareSignalJetState(signal);
    }
}

/**
 * @brief Parses a component to identify its input ports and create Json representation for each of them, which then are published
 * as individual Jet states.
 * 
 * @param functionBlock Object which contains input ports.
 * @param parentJsonValue Json object to which input port names are appended.
 */
void JetServer::parseComponentInputPorts(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue)
{
    const auto inputPorts = functionBlock.getInputPorts();

    // Add an empty Json entry if the object doesn't have input ports
    if(inputPorts.getCount() == 0)
    {
        parentJsonValue["InputPorts"] = Json::ValueType::nullValue;
        return;
    }

    for(auto inputPort : inputPorts) {
        std::string inputPortName = inputPort.getName();
        parentJsonValue["InputPorts"].append(inputPortName); // Add a list entry to the parent object consisting of input port names

        prepareInputPortJetState(inputPort);
    }
}

/**
 * @brief Determines type of an openDAQ property in order to correctly represent it in Json format.
 * 
 * @tparam PropertyHolder Type of the object which owns the property.
 * @param propertyHolder Object which owns the property.
 * @param property The property whose type is determined.
 * @param parentJsonValue Json object object to which the property is appended.
 */
template <typename PropertyHolder>
void JetServer::determinePropertyType(const PropertyHolder& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
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
                    determinePropertyType<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
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

// TODO! arguments are not received from jet, need to find out why and fix
/**
 * @brief Creates a callable Jet object which calls an openDAQ function or procedure.
 * 
 * @param propertyPublisher Component which has a callable property.
 * @param property Callable property.
 */
void JetServer::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
{
    std::string path = propertyPublisher.getGlobalId() + "/" + property.getName();

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

/**
 * @brief Appends simple propertiy types such as BoolProperty, IntProperty, FloatProperty and StringProperty to Json object,
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
 * @brief Appends ListProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendListProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    ListPtr<IBaseObject> opendaqList = propertyHolder.getPropertyValue(propertyName);
    CoreType listItemType = property.getItemType();

    Json::Value jsonArray = convertOpendaqListToJsonArray(opendaqList, listItemType);
    parentJsonValue[propertyName] = jsonArray;
}

/**
 * @brief Appends DictProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
 */
template<typename PropertyHolderType>
void JetServer::appendDictProperty(const PropertyHolderType& propertyHolder, const PropertyPtr& property, Json::Value& parentJsonValue)
{
    std::string propertyName = property.getName();
    DictPtr<IString, IBaseObject> opendaqDict = propertyHolder.getPropertyValue(propertyName); //! Non-string key types cannot be represented in Json::Value!
    CoreType itemCoreType = property.getItemType();

    Json::Value jsonDict = convertOpendaqDictToJsonDict(opendaqDict, itemCoreType);
    parentJsonValue[propertyName] = jsonDict;
}

/**
 * @brief Appends RatioProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json object to which the property is appended.
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

// TODO! This function is not tested
/**
 * @brief Appends ComplexNumber object to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param propertyName Name of the property.
 * @param parentJsonValue Json object to which the property is appended.
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

// TODO! This function needs to be finished
/**
 * @brief Appends StructProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
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
                            break;
                        case CoreType::ctFloat:
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
            case CoreType::ctStruct:
                break;
            case CoreType::ctEnumeration:
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
 * @brief Appends ObjectProperty to a Json object in order to be represented in a Jet state.
 * 
 * @tparam PropertyHolderType Type of the object which owns the property.
 * @param propertyHolder An object which owns the property.
 * @param property The property which is appended to a Jet state.
 * @param parentJsonValue Json object to which the property is appended.
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
        determinePropertyType<PropertyObjectPtr>(propertyObject, property, parentJsonValue[propertyObjectName]);
    }
}

/**
 * @brief Appends device metadata information to a Json object which is published as a Jet state. 
 * 
 * @param device Device from which metadata is retrieved.
 * @param parentJsonValue Json object to which metadata is appended.
 */
void JetServer::appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue)
{
    auto deviceInfo = device.getInfo();
    auto deviceInfoProperties = deviceInfo.getAllProperties();
    for(auto property : deviceInfoProperties) 
    {
        determinePropertyType<DeviceInfoPtr>(deviceInfo, property, parentJsonValue);
    }
}

/**
 * @brief Appends device domain data (e.g. time domain information) to a Json object which is published as a Jet state. 
 * 
 * @param device Device from which domain data is retrieved.
 * @param parentJsonValue Json object to which domain data is appended.
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
 * @brief Appends FunctionBlockInfo to a Json object which is published as a Jet state. 
 * FunctionBlockInfo is an information structure which contains metadata of the function block type.
 * 
 * @param functionBlock Function block from which FunctionBlockInfo structure is retrieved. Channel is a function block as well.
 * @param parentJsonValue  Json object to which FunctionBlockInfo structure is appended.
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
 * @brief Appends Global ID (global unique identified of the object) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which Global ID is retrieved.
 * @param parentJsonValue Json object to which Global ID is appended.
 */
void JetServer::appendGlobalId(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string globalId = component.getGlobalId();
    parentJsonValue["Global ID"] = globalId;
}

/**
 * @brief Appends type of the object (e.g. Device, Channel...) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which type is retrieved.
 * @param parentJsonValue Json object to which component's type is appended.
 */
void JetServer::appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    ConstCharPtr objectType = component.asPtr<ISerializable>().getSerializeId();
    parentJsonValue["_type"] = objectType;
}

/**
 * @brief Appends a component's activity status (true or false) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which activity status is retrieved.
 * @param parentJsonValue Json object to which activity status is appended.
 */
void JetServer::appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    bool isActive = component.getActive();
    parentJsonValue["Active"] = isActive;
}

/**
 * @brief Appends a component's visibility status (true or false) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which visibility status is retrieved.
 * @param parentJsonValue Json object to which visibility status is appended.
 */
void JetServer::appendVisibleStatus(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    bool isVisible = component.getVisible();
    parentJsonValue["Visible"] = isVisible;
}

/**
 * @brief Appends tags (user definable labels) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which tags are retrieved.
 * @param parentJsonValue Json object to which tags are appended.
 */
void JetServer::appendTags(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    TagsPtr tags = component.getTags();
    auto tagsList = tags.getList();

    // If there are no tags, still add "Tags" attribute to Json object with null value
    if(tagsList.getCount() == 0) {
        parentJsonValue["Tags"] = Json::ValueType::nullValue;
        return;
    }
    
    for(const std::string& item : tagsList)
        parentJsonValue["Tags"].append(item);
    
}

/**
 * @brief Addresses to a property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void JetServer::OPENDAQ_EVENT_updateProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    CoreType propertyType = component.getProperty(propertyName).getValueType();

    switch(propertyType) {
        case CoreType::ctBool:
            OPENDAQ_EVENT_updateSimpleProperty<bool>(component, eventParameters);
            break;
        case CoreType::ctInt:
            OPENDAQ_EVENT_updateSimpleProperty<int64_t>(component, eventParameters);
            break;
        case CoreType::ctFloat:
            OPENDAQ_EVENT_updateSimpleProperty<double>(component, eventParameters);
            break;
        case CoreType::ctString:
            OPENDAQ_EVENT_updateSimpleProperty<std::string>(component, eventParameters);
            break;
        case CoreType::ctList:
            OPENDAQ_EVENT_updateListProperty(component, eventParameters);
            break;
        case CoreType::ctDict:
            OPENDAQ_EVENT_updateDictProperty(component, eventParameters);
            break;
        case CoreType::ctRatio:
            break;
        case CoreType::ctComplexNumber:
            break;
        case CoreType::ctStruct:
            break;
        case CoreType::ctObject:
            break;
        case CoreType::ctProc:
            break;
        case CoreType::ctFunc:
            break;
        default:
            break;
    }

}

/**
 * @brief Addresses to a simple property (BoolProperty, IntProperty, FloatProperty and StringProperty) value change initiated 
 * by openDAQ client/server.
 * 
 * @tparam DataType Type of the property (bool, int64_t, double or std::string).
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
template <typename DataType>
void JetServer::OPENDAQ_EVENT_updateSimpleProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    DataType propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    jetState[propertyPath + propertyName] = propertyValue;
    jetPeer->notifyState(path, jetState);
}

/**
 * @brief Addresses to a list property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void JetServer::OPENDAQ_EVENT_updateListProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    ListPtr<IBaseObject> propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    CoreType listItemType = component.getProperty(propertyName).getItemType();
    Json::Value newPropertyValue = convertOpendaqListToJsonArray(propertyValue, listItemType);

    jetState[propertyPath + propertyName] = newPropertyValue;
    jetPeer->notifyState(path, jetState);
}

/**
 * @brief Addresses to a dict property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void JetServer::OPENDAQ_EVENT_updateDictProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    DictPtr<IString, IBaseObject> propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    CoreType dictItemType = component.getProperty(propertyName).getItemType();
    Json::Value newPropertyValue = convertOpendaqDictToJsonDict(propertyValue, dictItemType);

    jetState[propertyPath + propertyName] = newPropertyValue;
    jetPeer->notifyState(path, jetState);
}

/**
 * @brief Addresses to a "Active" status change initiated by openDAQ client/server.
 * 
 * @param component Component whose "Active" status is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void JetServer::OPENDAQ_EVENT_updateActiveStatus(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = readJetState(path);

    bool newActiveStatus = eventParameters.get("Active");

    jetState["Active"] = newActiveStatus;
    jetPeer->notifyState(path, jetState);
}

/**
 * @brief Addresses to a property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue Json object representing new value of the property.
 */
void JetServer::JET_EVENT_updateProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newPropertyValue)
{
    CoreType propertyType = component.getProperty(propertyName).getValueType();

    switch(propertyType) {
        case CoreType::ctBool:
            JET_EVENT_updateSimpleProperty<bool>(component, propertyName, newPropertyValue.asBool());
            break;
        case CoreType::ctInt:
            JET_EVENT_updateSimpleProperty<int64_t>(component, propertyName, newPropertyValue.asInt64());
            break;
        case CoreType::ctFloat:
            JET_EVENT_updateSimpleProperty<double>(component, propertyName, newPropertyValue.asDouble());
            break;
        case CoreType::ctString:
            JET_EVENT_updateSimpleProperty<std::string>(component, propertyName, newPropertyValue.asString());
            break;
        case CoreType::ctList:
            JET_EVENT_updateListProperty(component, propertyName, newPropertyValue);
            break;
        case CoreType::ctDict:
            JET_EVENT_updateDictProperty(component, propertyName, newPropertyValue);
            break;
        case CoreType::ctRatio:
            break;
        case CoreType::ctComplexNumber:
            break;
        case CoreType::ctStruct:
            break;
        case CoreType::ctObject:
            break;
        case CoreType::ctProc:
            break;
        case CoreType::ctFunc:
            break;
        default:
            break;
    }
}

/**
 * @brief Addresses to a simple property (BoolProperty, IntProperty, FloatProperty and StringProperty) value change initiated by a Jet peer.
 * 
 * @tparam DataType Type of the property (bool, int64_t, double or std::string).
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue New value of the property received from Jet.
 */
template <typename DataType>
void JetServer::JET_EVENT_updateSimpleProperty(const ComponentPtr& component, const std::string& propertyName, const DataType& newPropertyValue)
{
    component.setPropertyValue(propertyName, newPropertyValue);
}

/**
 * @brief Addresses to a list property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newJsonArray Json object containing new value of the list property.
 */
void JetServer::JET_EVENT_updateListProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonArray)
{
    ListPtr<IBaseObject> newOpendaqList = convertJsonArrayToOpendaqList(newJsonArray);
    component.setPropertyValue(propertyName, newOpendaqList);
}

/**
 * @brief Addresses to a dict property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newJsonDict Json object containing new value of the dict property.
 */
void JetServer::JET_EVENT_updateDictProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonDict)
{
    DictPtr<IString, IBaseObject> newOpendaqDict = convertJsonDictToOpendaqDict(newJsonDict);
    component.setPropertyValue(propertyName, newOpendaqDict);
}

/**
 * @brief Addresses to "Active" status change initiated by a Jet peer.
 * 
 * @param component Component whose "Active" status is changed.
 * @param newActiveStatus Json object containing new value for "Active" status.
 */
void JetServer::JET_EVENT_updateActiveStatus(const ComponentPtr& component, const Json::Value& newActiveStatus)
{
    component.setActive(newActiveStatus.asBool());
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
