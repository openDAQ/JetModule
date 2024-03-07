#include "component_converter.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

ComponentConverter::ComponentConverter(const InstancePtr& opendaqInstance) : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    this->opendaqInstance = opendaqInstance;
    // initiate openDAQ logger
    logger = LoggerComponent("ObjectConverterLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

/**
 * @brief Composes Json representation of an openDAQ component and publishes it as Jet state.
 * This function is overriden by every Converter class in order to convert different openDAQ objects according to the data they host.
 * 
 * @param component OpenDAQ component which has to be converted into its Json representation.
 */
void ComponentConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendVisibleStatus(component, jetState);
    appendTags(component, jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

/**
 * @brief Defines a callback function for a component which will be called when some change occurs in a structure
 * of an openDAQ component.
 * 
 * @param component Pointer to the component for which a callback function is defined.
 */
void ComponentConverter::createOpendaqCallback(const ComponentPtr& component)
{
    component.getOnComponentCoreEvent() += [this](const ComponentPtr& comp, const CoreEventArgsPtr& args)
    {
        DictPtr<IString, IBaseObject> eventParameters = args.getParameters();
        if(eventParameters.hasKey("Name")) { // Property changed
            opendaqEventHandler.updateProperty(comp, eventParameters);
        }
        else if(eventParameters.hasKey("Active")) { // Active status changed
            opendaqEventHandler.updateActiveStatus(comp, eventParameters);
        }
        else if(eventParameters.hasKey("Property")) { // Property added
            opendaqEventHandler.addProperty(comp, eventParameters);
        }
        else {
            std::string message = "Unknown change occured to component \"" + comp.getName() + "\"\n";
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
        }
    };
}

/**
 * @brief Defines a callback function for a Jet state which will be called when some change occurs in that Jet state.
 * 
 * @return JetStateCallback callback function which will be called during the change from Jet. It has to be passed to Jet state publisher.
 */
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
 * in the component's Jet state.
 * 
 * @param component Component which is parsed to retrieve its properties.
 * @param parentJsonValue Json object which is filled with representations of openDAQ properties.
 */
void ComponentConverter::appendProperties(const ComponentPtr& component, Json::Value& parentJsonValue)
{
    std::string propertyPublisherName = component.getName();

    auto properties = component.getAllProperties();
    for(auto property : properties) {
        propertyManager.determinePropertyType<ComponentPtr>(component, property, parentJsonValue);
    }
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