#pragma once
#include <jet_module/common.h>
#include <opendaq/module_impl.h>
#include <iostream>

BEGIN_NAMESPACE_JET_MODULE

class JetModule final : public Module
{
public:
    JetModule(ContextPtr context);

    // DictPtr<IString, IServerType> onGetAvailableServerTypes() override;
    // ServerPtr onCreateServer(StringPtr serverType, PropertyObjectPtr serverConfig, DevicePtr rootDevice) override;

private:
    std::mutex sync;
};


END_NAMESPACE_JET_MODULE