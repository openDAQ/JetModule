#include <utility>
#include <coretypes/version_info_factory.h>

#include "version.h"
#include "jet_module_impl.h"


BEGIN_NAMESPACE_JET_MODULE

JetModule::JetModule(ContextPtr context)
    : Module("Jet module",
             daq::VersionInfo(JET_MODULE_MAJOR_VERSION, JET_MODULE_MINOR_VERSION,JET_MODULE_PATCH_VERSION),
             std::move(context)
    )
{
}

END_NAMESPACE_JET_MODULE
