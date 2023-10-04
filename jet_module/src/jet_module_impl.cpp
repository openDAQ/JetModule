// #include <jet_module/jet_impl.h>
#include <jet_module/jet_module_impl.h>
#include <jet_module/version.h>
#include <coretypes/version_info_factory.h>
#include <chrono>

BEGIN_NAMESPACE_JET_MODULE

JetModule::JetModule(ContextPtr context)
    : Module("Jet module",
             daq::VersionInfo(JET_MODULE_MAJOR_VERSION, JET_MODULE_MINOR_VERSION,JET_MODULE_PATCH_VERSION),
             std::move(context))
{
}

// DictPtr<IString, IServerType> JetModule::onGetAvailableServerTypes()
// {
//     auto result = Dict<IString, IServerType>();

//     auto serverType = JetModule::createType();
//     result.set(serverType.getId(), serverType);

//     return result;
// }

// ServerPtr JetModule::onCreateServer(StringPtr serverType,
//                                          PropertyObjectPtr serverConfig,
//                                          DevicePtr rootDevice)
// {
//     if (!context.assigned())
//         throw InvalidParameterException{"Context parameter cannot be null."};

//     if (!serverConfig.assigned())
//         serverConfig = JetModule::createDefaultConfig();

//     ServerPtr server(Jet_Create(rootDevice, serverConfig, context));
//     return server;
// }

END_NAMESPACE_JET_MODULE
