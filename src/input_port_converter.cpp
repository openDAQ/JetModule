#include "input_port_converter.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Composes Json representation of an openDAQ input port and publishes it as Jet state.
 * This function is overriden by every Converter class in order to convert different openDAQ objects according to the data they host.
 * 
 * @param component OpenDAQ input port which has to be converted into its Json representation.
 */
void InputPortConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendVisibleStatus(component, jetState);
    appendTags(component, jetState);    

    appendInputPortInfo(component.asPtr<IInputPort>(), jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

/**
 * @brief Parses an input port and prepares its Json representation for publishing as a Jet state.
 * 
 * @param inputPort Pointer to the input port which is parsed.
 */

/**
 * @brief Parses an input port and prepares its Json representation for publishing as a Jet state.
 * 
 * @param inputPort Pointer to the input port which is parsed.
 * @param parentJsonValue Json object to which input port metadata is appended.
 */
void InputPortConverter::appendInputPortInfo(const InputPortPtr& inputPort, Json::Value& parentJsonValue)
{
    std::string inputPortName = inputPort.getName();

    bool requiresSignal = inputPort.getRequiresSignal();
    parentJsonValue["RequiresSignal"] = requiresSignal;
}

END_NAMESPACE_JET_MODULE