#include "jet_server.h"
#include "jet_module_exceptions.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Constructs a new Jet Server object. It takes an openDAQ device as an argument and publishes its tree structure in Json representation
 * as Jet states.
 * 
 * @param device A device which will be parsed and structure of which is published as Jet states.
 */
JetServer::JetServer(const InstancePtr& instance)
    : 
    componentConverter(instance),
    deviceConverter(instance),
    functionBlockConverter(instance),
    channelConverter(instance),
    signalConverter(instance),
    inputPortConverter(instance)
{
    this->opendaqInstance = instance;
    this->rootDevice = instance.getRootDevice();
}

JetServer::~JetServer()
{
}

/**
 * @brief Publishes a device's tree structure in Json format as Jet states.
 * 
 */
void JetServer::publishJetStates()
{
    // Have to parse root device separately because parsing in parseOpendaqInstance function is done relative to it
    deviceConverter.composeJetState(rootDevice);
    parseOpendaqInstance(opendaqInstance);
}

/**
 * @brief Parses a openDAQ folder to identify components in it. The components are parsed themselves to create their Jet states.
 * 
 * @param parentFolder A folder which is parsed to identify components in it.
 */
void JetServer::parseOpendaqInstance(const FolderPtr& parentFolder)
{
    auto items = parentFolder.getItems(search::Any());
    for(const auto& item : items)
    {
        auto folder = item.asPtrOrNull<IFolder>();
        auto component = item.asPtrOrNull<IComponent>();
        auto device = item.asPtrOrNull<IDevice>();
        auto functionBlock = item.asPtrOrNull<IFunctionBlock>();
        auto channel = item.asPtrOrNull<IChannel>();
        auto signal = item.asPtrOrNull<ISignal>();
        auto inputPort = item.asPtrOrNull<IInputPort>();

        if(device.assigned()) {
            deviceConverter.composeJetState(device);
        }
        else if(channel.assigned()) {
            channelConverter.composeJetState(channel);
        }
        else if(functionBlock.assigned()) {
            functionBlockConverter.composeJetState(functionBlock);
        }
        else if(signal.assigned()) {
            signalConverter.composeJetState(signal);
        }
        else if(inputPort.assigned()) {
            inputPortConverter.composeJetState(inputPort);
        }
        else if(folder.assigned()) { // It is important to test for folder last as everything besides component is a folder as well
            // We do nothing here because we want to identify pure components (not its descendants)
            // Recursion is done in separate if statement
        }
        else if(component.assigned()) { // It is important to test for component after folder!
            componentConverter.composeJetState(component);
        }
        else {
            // throwJetModuleException(JM_UNSUPPORTED_ITEM);
        }

        if(folder.assigned()) {
            parseOpendaqInstance(folder);
        }
    }
}

END_NAMESPACE_JET_MODULE
