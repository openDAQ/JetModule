#include "property_manager.h"
#include <opendaq/logger_component_factory.h>

BEGIN_NAMESPACE_JET_MODULE

PropertyManager::PropertyManager() : jetPeerWrapper(JetPeerWrapper::getInstance())
{
    // initiate openDAQ logger
    logger = LoggerComponent("PropertyManagerLogger", DefaultSinks(), LoggerThreadPool(), LogLevel::Default);
}

// TODO! arguments are not received from jet, need to find out why and fix
/**
 * @brief Creates a callable Jet object which calls an openDAQ function or procedure.
 * 
 * @param propertyPublisher Component which has a callable property.
 * @param property Callable property.
 */
void PropertyManager::createJetMethod(const ComponentPtr& propertyPublisher, const PropertyPtr& property)
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

END_NAMESPACE_JET_MODULE