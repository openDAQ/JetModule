#include "jet_event_handler.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

JetEventHandler::JetEventHandler() : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    // initiate openDAQ logger
    logger = LoggerComponent("JetEventHandlerLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

/**
 * @brief Addresses to a property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue Json object representing new value of the property.
 */
void JetEventHandler::updateProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newPropertyValue)
{
    CoreType propertyType = component.getProperty(propertyName).getValueType();

    std::string message = "Update of property with CoreType " + propertyType + std::string(" is not supported currently.\n");

    switch(propertyType) {
        case CoreType::ctBool:
            updateSimpleProperty<bool>(component, propertyName, newPropertyValue.asBool());
            break;
        case CoreType::ctInt:
            updateSimpleProperty<int64_t>(component, propertyName, newPropertyValue.asInt64());
            break;
        case CoreType::ctFloat:
            updateSimpleProperty<double>(component, propertyName, newPropertyValue.asDouble());
            break;
        case CoreType::ctString:
            updateSimpleProperty<std::string>(component, propertyName, newPropertyValue.asString());
            break;
        case CoreType::ctList:
            updateListProperty(component, propertyName, newPropertyValue);
            break;
        case CoreType::ctDict:
            updateDictProperty(component, propertyName, newPropertyValue);
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
 * @brief Addresses to a simple property (BoolProperty, IntProperty, FloatProperty and StringProperty) value change initiated by a Jet peer.
 * 
 * @tparam DataType Type of the property (bool, int64_t, double or std::string).
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newPropertyValue New value of the property received from Jet.
 */
template <typename DataType>
void JetEventHandler::updateSimpleProperty(const ComponentPtr& component, const std::string& propertyName, const DataType& newPropertyValue)
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
void JetEventHandler::updateListProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonArray)
{
    ListPtr<IBaseObject> newOpendaqList = propertyConverter.convertJsonArrayToOpendaqList(newJsonArray);
    component.setPropertyValue(propertyName, newOpendaqList);
}

/**
 * @brief Addresses to a dict property value change initiated by a Jet peer.
 * 
 * @param component Component whose property value is changed.
 * @param propertyName Name of the property.
 * @param newJsonDict Json object containing new value of the dict property.
 */
void JetEventHandler::updateDictProperty(const ComponentPtr& component, const std::string& propertyName, const Json::Value& newJsonDict)
{
    DictPtr<IString, IBaseObject> newOpendaqDict = propertyConverter.convertJsonDictToOpendaqDict(newJsonDict);
    component.setPropertyValue(propertyName, newOpendaqDict);
}

/**
 * @brief Addresses to "Active" status change initiated by a Jet peer.
 * 
 * @param component Component whose "Active" status is changed.
 * @param newActiveStatus Json object containing new value for "Active" status.
 */
void JetEventHandler::updateActiveStatus(const ComponentPtr& component, const Json::Value& newActiveStatus)
{
    component.setActive(newActiveStatus.asBool());
}

END_NAMESPACE_JET_MODULE