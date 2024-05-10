#include "property_manager.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

PropertyManager::PropertyManager() : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    
}

/**
 * @brief Creates a callable Jet object which calls an openDAQ function or procedure.
 * 
 * @param propertyPublisher Component which has a callable property.
 * @param property Callable property.
 */
void PropertyManager::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
{
    std::string propertyName = property.getName();
    std::string path = propertyPublisher.getGlobalId() + "/" + propertyName;

    BaseObjectPtr func = propertyPublisher.getPropertyValue(propertyName);
    CallableInfoPtr callableInfo = property.getCallableInfo();
    CoreType funcType = property.getValueType();

    if(hasUnsupportedArgument(callableInfo, propertyName)) {
        DAQLOG_E(jetModuleLogger, jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_TYPE).c_str());
        return;
    }
    // We have to check whether IFunction has a valid return type. IProcedure does not return anything
    if(funcType == ctFunc && hasUnsupportedReturnType(callableInfo.getReturnType(), propertyName)) {
        DAQLOG_E(jetModuleLogger, jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_RETURN_TYPE).c_str());
        return;
    }

    auto cb = [func, callableInfo, funcType, this](const Json::Value& args) -> Json::Value
    {
        size_t numberOfArgs;
        auto funcArgs = callableInfo.getArguments();
        if(funcArgs == nullptr) // Function has no arguments
            numberOfArgs = 0;
        else    
            numberOfArgs = funcArgs.getCount();

        BaseObjectPtr returnValue; // For ctFunc. ctProc doesn't have a return value

        // Function with zero arguments
        if(numberOfArgs == 0 && args.size() == 0) {
            if(funcType == CoreType::ctProc)
                func.asPtr<IProcedure>()();
            else if(funcType == CoreType::ctFunc)
                returnValue = func.asPtr<IFunction>()();
            else
                return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);

        }
        // Function with one argument
        else if(args.isNumeric() || args.isBool() || args.isString() && numberOfArgs == 1) {
            if(!hasCompatibleArgumentTypes(funcArgs[0].getType(), args))
                return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_INCOMPATIBLE_ARGUMENT_TYPES);
            
            BaseObjectPtr daqArg = convertJsonValueToDaqValue(args);
            if(daqArg == nullptr)
                return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_FORMAT);

            if(funcType == CoreType::ctProc)
                func.asPtr<IProcedure>()(daqArg);
            else if(funcType == CoreType::ctFunc)
                returnValue = func.asPtr<IFunction>()(daqArg);
            else
                return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);

            // A function can be called this way as well
            // func.dispatch(daqArg); 
        }
        else if(args.isArray()) {
            // Function with one arguments provided as a Json list
            if(numberOfArgs == 1 && args.size() == 1) {
                if(!hasCompatibleArgumentTypes(funcArgs[0].getType(), args[0]))
                    return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_INCOMPATIBLE_ARGUMENT_TYPES);
                
                BaseObjectPtr daqArg = convertJsonValueToDaqValue(args[0]);
                if(daqArg == nullptr)
                    return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_FORMAT);

                if(funcType == CoreType::ctProc)
                    func.asPtr<IProcedure>()(daqArg);
                else if(funcType == CoreType::ctFunc)
                    returnValue = func.asPtr<IFunction>()(daqArg);
                else
                    return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);
            }
            // Function with multiple arguments
            else if(numberOfArgs == args.size()) {
                auto list = List<IBaseObject>();
                int i = 0;
                for(const auto& arg : args) {
                    if(!hasCompatibleArgumentTypes(funcArgs[i].getType(), arg))
                        return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_INCOMPATIBLE_ARGUMENT_TYPES);
                    
                    BaseObjectPtr daqArg = convertJsonValueToDaqValue(arg);
                    if(daqArg == nullptr)
                        return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_FORMAT);
                            
                    list.pushBack(daqArg);
                    i++;
                }
                if(funcType == CoreType::ctProc)
                    func.asPtr<IProcedure>()(list);
                else if(funcType == CoreType::ctFunc)
                    returnValue = func.asPtr<IFunction>()(list);
                else
                    return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);
            }
            // Number of arguments for the function don't match to arguments provided from Jet
            else {
                return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_INCORRECT_ARGUMENT_NUMBER);
            }
        }
        else {
            return jetModuleExceptionToString(JetModuleException::JM_FUNCTION_UNSUPPORTED_ARGUMENT_FORMAT);
        }
            
        if(funcType == CoreType::ctProc)
            return "Procedure called successfully!";
        else if(funcType == CoreType::ctFunc) {
                Json::Value returnValJson = convertDaqValueToJsonValue(returnValue, callableInfo.getReturnType());
                if(returnValJson.type() != Json::ValueType::nullValue)
                    return returnValJson;
                else
                    return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);
            }
        else
            return jetModuleExceptionToString(JetModuleException::JM_UNEXPECTED_TYPE);
    };

    jetPeerWrapper.publishJetMethod(path, cb);
}

/**
 * @brief Convert Json value to openDAQ value represented BaseObject pointer. This function support conversion of 
 * only simple types.
 * 
 * @param jsonVal value in Json.
 * @return openDAQ value represented as BaseObjectPtr. nullptr is returned in case of unsupported Json types.
 */
BaseObjectPtr PropertyManager::convertJsonValueToDaqValue(const Json::Value& jsonVal)
{
    if(jsonVal.isBool())
        return jsonVal.asBool();
    else if(jsonVal.isInt())
        return jsonVal.asInt();
    else if(jsonVal.isInt64())
        return jsonVal.asInt64();
    else if(jsonVal.isUInt())
        return jsonVal.asUInt();
    else if(jsonVal.isUInt64())
        return jsonVal.asUInt64();
    else if(jsonVal.isDouble())
        return jsonVal.asDouble();
    else if(jsonVal.isString())
        return jsonVal.asString();
    else
        return nullptr;
}

/**
 * @brief Converts openDAQ value to Json value. Supports conversion of simple openDAQ types.
 * 
 * @param daqVal OpenDAQ value.
 * @param coretype Type of the openDAQ value.
 * @return Value in Json. In case of unsupported types Json value initialized with nullValue is returned.
 */
Json::Value PropertyManager::convertDaqValueToJsonValue(const BaseObjectPtr& daqVal, const CoreType& coretype)
{
    switch(coretype)
    {
        case CoreType::ctBool:
            return static_cast<bool>(daqVal.asPtr<IBoolean>());
        case CoreType::ctInt:
            return static_cast<int64_t>(daqVal.asPtr<IInteger>());
        case CoreType::ctFloat:
            return static_cast<double>(daqVal.asPtr<IFloat>());
        case CoreType::ctString:
            return static_cast<std::string>(daqVal.asPtr<IString>());
        default:
            return Json::Value(Json::nullValue);
    }
}

/**
 * @brief Checks whether a function or a procedure has arguments with unsupported types.
 * 
 * @param callableInfo An object containing information on a function/procedure's arguments' types.
 * @param propertyName Name of the callable property.
 * @return true if the function/procedure has unsupported arguments.
 * @return false if the function does not have unsupported arguments.
 */
bool PropertyManager::hasUnsupportedArgument(const CallableInfoPtr& callableInfo, const std::string& propertyName)
{
    auto funcArgs = callableInfo.getArguments();
    if(funcArgs == nullptr) // function has no arguments
        return false; // false means that everything is ok. true - function has an unsupported argument
    else {
        for(const auto& arg : funcArgs) {
            CoreType argType = arg.getType();
            if(!(argType == ctBool || argType == ctInt || argType == ctFloat || argType == ctString)) { // These are the supported types of arguments for a function
                std::string message = "Unable to add FunctionProperty \"" + propertyName + "\" because of unsupported argument. Supported function arguments are: ctBool, ctInt, ctFloat, ctString.";
                DAQLOG_E(jetModuleLogger, message.c_str());
                return true;
            }
        }
        return false;
    }
}

/**
 * @brief Checks whether a function returns unsupported variable type.
 * 
 * @param returnType Type of the return value.
 * @param propertyName Name of the callable property.
 * @return true if the function's return type is unsupported.
 * @return false if the function's return type is supported.
 */
bool PropertyManager::hasUnsupportedReturnType(const CoreType& returnType, const std::string& propertyName)
{
    if(!(returnType == ctBool || returnType == ctInt || returnType == ctFloat || returnType == ctString)) { // These are the supported return typess for a function
        std::string message = "Unable to add FunctionProperty \"" + propertyName + "\" because of unsupported return type. Supported function return types are: ctBool, ctInt, ctFloat, ctString.";
        DAQLOG_E(jetModuleLogger, message.c_str());
        return true;
    }
    else
        return false;
}

/**
 * @brief Checks whether a function's/procedure's arguments have been provided with correct types from Jet.
 * 
 * @param daqType Type of the function's/procedure's argument in openDAQ.
 * @param jsonVal Type of the function's/procedure's argument provided from Jet.
 * @return true if the argument types provided from Jet are compatible with function's/procedure's definition.
 * @return false if the argument types provided from Jet are incompatible with function's/procedure's definition.
 */
bool PropertyManager::hasCompatibleArgumentTypes(CoreType daqType, const Json::Value& jsonVal)
{
    switch(daqType)
    {
        case CoreType::ctBool:
            if(jsonVal.isBool()) return true;
            break;
        case CoreType::ctInt:
            if(jsonVal.isInt() || jsonVal.isInt64() || jsonVal.isUInt() || jsonVal.isUInt64()) return true;
            break;
        case CoreType::ctFloat:
            if(jsonVal.isDouble()) return true;
            break;
        case CoreType::ctString:
            if(jsonVal.isString()) return true;
            break;
        default:
            // For unsupported daq types 'false' is returned
            return false;
            break;
    }

    return false;
}

END_NAMESPACE_JET_MODULE