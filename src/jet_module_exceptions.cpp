#include <iostream>
#include <jet/defines.h>
#include "jet_module_exceptions.h"

using namespace daq;

bool checkTypeCompatibility(Json::ValueType jsonValueType, daq::CoreType daqValueType)
{
    switch(jsonValueType)
    {
        case Json::ValueType::intValue:
            {
                if(daqValueType == CoreType::ctInt || daqValueType == CoreType::ctFloat)
                    return true;
                else
                    return false;
            }
            break;
        case Json::ValueType::uintValue:
            {
                if(daqValueType == CoreType::ctInt || daqValueType == CoreType::ctFloat)
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
        case Json::ValueType::objectValue:
            {
                if(daqValueType == CoreType::ctObject)
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

void throwJetModuleException(JetModuleException jmException)
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

void throwJetModuleException(JetModuleException jmException, std::string propertyName)
{
    switch(jmException)
    {
        case JetModuleException::JM_INCOMPATIBLE_TYPES:
            {
                std::string message = "Incorrect type detected for openDAQ property: " + propertyName;
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

void throwJetModuleException(JetModuleException jmException, Json::ValueType jsonValueType, std::string propertyName, std::string globalId)
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