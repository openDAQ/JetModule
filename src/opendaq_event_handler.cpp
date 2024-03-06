#include "opendaq_event_handler.h"
#include <json/value.h>
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

OpendaqEventHandler::OpendaqEventHandler() : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    // initiate openDAQ logger
    logger = LoggerComponent("OpendaqEventHandlerLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

/**
 * @brief Addresses to a property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string propertyName = eventParameters.get("Name");
    CoreType propertyType = component.getProperty(propertyName).getValueType();

    std::string message = "Update of property with CoreType " + propertyType + std::string(" is not supported currently.\n");

    switch(propertyType) {
        case CoreType::ctBool:
            updateSimpleProperty<bool>(component, eventParameters);
            break;
        case CoreType::ctInt:
            updateSimpleProperty<int64_t>(component, eventParameters);
            break;
        case CoreType::ctFloat:
            updateSimpleProperty<double>(component, eventParameters);
            break;
        case CoreType::ctString:
            updateSimpleProperty<std::string>(component, eventParameters);
            break;
        case CoreType::ctList:
            updateListProperty(component, eventParameters);
            break;
        case CoreType::ctDict:
            updateDictProperty(component, eventParameters);
            break;
        case CoreType::ctRatio:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
            break;
        case CoreType::ctComplexNumber:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
            break;
        case CoreType::ctStruct:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
            break;
        case CoreType::ctObject:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
            break;
        case CoreType::ctProc:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
            break;
        case CoreType::ctFunc:
            logger.logMessage(SourceLocation{__FILE__, __LINE__, OPENDAQ_CURRENT_FUNCTION}, message.c_str(), LogLevel::Warn);
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
void OpendaqEventHandler::updateSimpleProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    DataType propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    jetState[propertyPath + propertyName] = propertyValue;
    jetPeerWrapper.updateJetState(path, jetState);
}

/**
 * @brief Addresses to a list property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateListProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    ListPtr<IBaseObject> propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    CoreType listItemType = component.getProperty(propertyName).getItemType();
    Json::Value newPropertyValue = propertyConverter.convertOpendaqListToJsonArray(propertyValue, listItemType);

    jetState[propertyPath + propertyName] = newPropertyValue;
    jetPeerWrapper.updateJetState(path, jetState);
}

/**
 * @brief Addresses to a dict property value change initiated by openDAQ client/server.
 * 
 * @param component Component whose property value is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateDictProperty(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    std::string propertyName = eventParameters.get("Name");
    DictPtr<IString, IBaseObject> propertyValue = eventParameters.get("Value");
    std::string propertyPath = eventParameters.get("Path");

    CoreType dictItemType = component.getProperty(propertyName).getItemType();
    Json::Value newPropertyValue = propertyConverter.convertOpendaqDictToJsonDict(propertyValue, dictItemType);

    jetState[propertyPath + propertyName] = newPropertyValue;
    jetPeerWrapper.updateJetState(path, jetState);
}

/**
 * @brief Addresses to a "Active" status change initiated by openDAQ client/server.
 * 
 * @param component Component whose "Active" status is changed.
 * @param eventParameters Dictionary filled with data describing the change.
 */
void OpendaqEventHandler::updateActiveStatus(const ComponentPtr& component, const DictPtr<IString, IBaseObject>& eventParameters)
{
    std::string path = component.getGlobalId();
    Json::Value jetState = jetPeerWrapper.readJetState(path);

    bool newActiveStatus = eventParameters.get("Active");

    jetState["Active"] = newActiveStatus;
    jetPeerWrapper.updateJetState(path, jetState);
}

END_NAMESPACE_JET_MODULE