#include "function_block_converter.h"

BEGIN_NAMESPACE_JET_MODULE

void FunctionBlockConverter::composeJetState(const ComponentPtr& component)
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

/**
 * @brief Appends FunctionBlockInfo to a Json object which is published as a Jet state. 
 * FunctionBlockInfo is an information structure which contains metadata of the function block type.
 * 
 * @param functionBlock Function block from which FunctionBlockInfo structure is retrieved. Channel is a function block as well.
 * @param parentJsonValue  Json object to which FunctionBlockInfo structure is appended.
 */
void FunctionBlockConverter::appendFunctionBlockInfo(const FunctionBlockPtr& functionBlock, Json::Value& parentJsonValue)
{
    const FunctionBlockTypePtr fbType = functionBlock.getFunctionBlockType();
    std::string fbId = fbType.getId();
    std::string fbName = fbType.getName();
    std::string fbDescription = fbType.getDescription();
    parentJsonValue["FunctionBlockInfo"]["Id"] = fbId;
    parentJsonValue["FunctionBlockInfo"]["Name"] = fbName;
    parentJsonValue["FunctionBlockInfo"]["Description"] = fbDescription;

}

END_NAMESPACE_JET_MODULE