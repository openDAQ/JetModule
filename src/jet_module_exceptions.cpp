#include <iostream>
#include <jet/defines.h>
#include "jet_module_exceptions.h"

BEGIN_NAMESPACE_JET_MODULE

using namespace daq;

LoggerComponentPtr jetModuleLogger = LoggerComponent("JetModule", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);

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
        case JetModuleException::JM_UNSUPPORTED_ITEM:
           {
                std::string message = "Unsupported openDAQ item";
                std::cout << "addJetState cb: " << message << std::endl;
                throw new hbk::jet::jsoncpprpcException(
                    JM_UNSUPPORTED_ITEM,                  // code
                    message                                 // message
                    // Json::Value()                        // data
                );
            }
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

std::string jetModuleExceptionToString(const JetModuleException& jmException)
{
    std::string message = "Error: ";
    switch(jmException)
    {
        case JetModuleException::JM_UNEXPECTED_TYPE:
            return (message + "Unexpected type detected.");
        case JetModuleException::JM_FUNCTION_INCOMPATIBLE_ARGUMENT_TYPES:
            return (message + "Incompatible function argument types detected.");
        case JetModuleException::JM_FUNCTION_INCORRECT_ARGUMENT_NUMBER:
            return (message + "Incorrect number of arguments has been provided.");
        case JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_TYPE:
            return (message + "Function is defined with an argument type which is not supported.");
        case JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_FORMAT:
            return (message + "Arguments to the function have been provided in unsupported format.");
        case JetModuleException::JM_FUNCTION_UNSUPPORTED_RETURN_TYPE:
            return (message + "Function is defined with a return type which is not supported.");
        default:
            return (message + "General error.");
    }
}

END_NAMESPACE_JET_MODULE