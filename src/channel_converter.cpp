#include "channel_converter.h"

BEGIN_NAMESPACE_JET_MODULE

void ChannelConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendTags(component, jetState);    

    appendFunctionBlockInfo(component, jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

END_NAMESPACE_JET_MODULE