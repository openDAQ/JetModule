#include "component_converter.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

ComponentConverter::ComponentConverter(const InstancePtr& opendaqInstance) : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    this->opendaqInstance = opendaqInstance;
    // initiate openDAQ logger
    logger = LoggerComponent("ObjectConverterLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

void ComponentConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendTags(component, jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

/**
 * @brief Defines a callback function for a component which will be called when some event occurs in a structure
 * of an openDAQ component.
 * 
 * @param component Pointer to the component for which a callback function is defined.
 */
void ComponentConverter::createOpendaqCallback(const ComponentPtr& component)
{
    component.getOnComponentCoreEvent() += [this](const ComponentPtr& comp, const CoreEventArgsPtr& args)
    {
        auto parameters = args.getParameters();
        auto keyList = parameters.getKeyList();
        auto valueList = parameters.getValueList();
        // std::cout << "Size = " << keyList.getCount() << std::endl;
        // for(int i = 0; i < keyList.getCount(); i++) {
        //     std::cout << "Key: " << keyList[i] << std::endl;
        //     std::cout << "Value: " << valueList[i] << std::endl;
        // }
        // std::cout << std::endl;

        DictPtr<IString, IBaseObject> eventParameters = args.getParameters();
        if(eventParameters.hasKey("Name")) { // Properties
            opendaqEventHandler.updateProperty(comp, eventParameters);
        }
        else if(eventParameters.hasKey("Active")) { // Active status
            opendaqEventHandler.updateActiveStatus(comp, eventParameters);
        }
    };
}

JetStateCallback ComponentConverter::createJetCallback()
{
    JetStateCallback callback = [this](const Json::Value& value, std::string path) -> Json::Value
    {
        std::string message = "Want to change state with path: " + path + " with the value " + value.toStyledString() + "\n";
        logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Info);
        
        // Actual work is done on a separate thread to handle simultaneous requests. Also, otherwise "jetset" tool would time out
        std::thread([this, value, path]() 
        {
            // We find component by searching relative to root device, so we have to remove its name from global ID of the component with provided path
            std::string relativePath = jetPeerWrapper.removeRootDeviceId(path);
            ComponentPtr component = opendaqInstance.findComponent(relativePath);

            Json::Value jetState = jetPeerWrapper.readJetState(path);

            for (auto it = value.begin(); it != value.end(); ++it) {
                std::string entryName = it.key().asString();
                Json::Value entryValue = *it;

                if(component.hasProperty(entryName)) {
                    jetEventHandler.updateProperty(component, entryName, entryValue);
                }
                else if(entryName == "Active") {
                    jetEventHandler.updateActiveStatus(component, entryValue);
                }
                else if(entryName == "Tags") {
                    // TODO: Implement a function which updates tags
                }
            }

        }).detach(); // detach is used to separate the thread of execution from the thread object, allowing execution to continue independently

        return Json::Value(); // Return an empty Json as there's no need to return anything specific.
        // TODO: Make sure that this is ok
    };

    return callback;
}

/**
 * @brief Parses a component to get its properties which are converted into Json representation in order to be published
 * in the component's Jet state
 * 
 * @param component Component which is parsed to retrieve its properties.
 * @param parentJsonValue Json object which is filled with representations of openDAQ properties.
 */
void ComponentConverter::appendProperties(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string propertyPublisherName = component.getName();

    auto properties = component.getAllProperties();
    for(auto property : properties) {
        determinePropertyType<ComponentPtr>(component, property, parentJsonValue);
    }
}

// TODO! arguments are not received from jet, need to find out why and fix
/**
 * @brief Creates a callable Jet object which calls an openDAQ function or procedure.
 * 
 * @param propertyPublisher Component which has a callable property.
 * @param property Callable property.
 */
void ComponentConverter::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
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
                        propertyConverter.convertJsonToDaqArguments(daqArg, args, i);
                    }
                }
                else
                {
                    propertyConverter.convertJsonToDaqArguments(daqArg, args, 0);
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

    jetPeerWrapper.publishJetMethod(path, cb);
}

/**
 * @brief Appends type of the object (e.g. Device, Channel...) to a Json object which is published as a Jet state.
 * 
 * @param component Component from which type is retrieved.
 * @param parentJsonValue Json object to which component's type is appended.
 */
void ComponentConverter::appendObjectType(const ComponentPtr& component, Json::Value& parentJsonValue)
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
void ComponentConverter::appendActiveStatus(const ComponentPtr& component, Json::Value& parentJsonValue)
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
void ComponentConverter::appendVisibleStatus(const ComponentPtr& component, Json::Value& parentJsonValue)
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
void ComponentConverter::appendTags(const ComponentPtr& component, Json::Value& parentJsonValue)
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

END_NAMESPACE_JET_MODULE