#include "signal_converter.h"
#include <iostream>

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Composes Json representation of an openDAQ signal and publishes it as Jet state.
 * This function is overriden by every Converter class in order to convert different openDAQ objects according to the data they host.
 * 
 * @param component OpenDAQ signal which has to be converted into its Json representation.
 */
void SignalConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the component to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a component's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendVisibleStatus(component, jetState);
    appendTags(component, jetState);    

    appendSignalInfo(component.asPtr<ISignal>(), jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

/**
 * @brief Appends a signal vlaue information according to its DataDescriptor to a Json value, which will be published as a Jet state.
 * 
 * @param signal OpenDAQ signal from which its DataDescriptor is retrieved.
 * @param parentJsonValue Json object to which signal metadata is appended.
 */
void SignalConverter::appendSignalInfo(const SignalPtr& signal, Json::Value& parentJsonValue)
{
    std::string signalName = signal.getName();
    DataDescriptorPtr dataDescriptor = signal.getDescriptor();

    // If data descriptor is empty add an empty Json enrtry
    if(dataDescriptor.assigned() == false) {
        parentJsonValue["Value"]["DataDescriptor"] = Json::ValueType::nullValue;
        return;
    }

    std::string name = dataDescriptor.getName();
        parentJsonValue["Value"]["DataDescriptor"]["Name"] = name;
    ListPtr<IDimension> dimensions = dataDescriptor.getDimensions();
        size_t dimensionsCount = dimensions.getCount();
        parentJsonValue["Value"]["DataDescriptor"]["Dimensions"] = dimensionsCount;
    DictPtr<IString, IString> metadata = dataDescriptor.getMetadata();
        size_t metadataCount = metadata.getCount();
        parentJsonValue["Value"]["DataDescriptor"]["Metadata"] = metadataCount;
    DataRulePtr rule = dataDescriptor.getRule();
    if(rule.assigned())
        parentJsonValue["Value"]["DataDescriptor"]["Rule"] = propertyConverter.convertDataRuleToJsonObject(rule);
    else
        parentJsonValue["Value"]["DataDescriptor"]["Rule"] = Json::ValueType::nullValue;
    SampleType sampleType = dataDescriptor.getSampleType();
        parentJsonValue["Value"]["DataDescriptor"]["SampleType"] = int(sampleType);
    UnitPtr unit = dataDescriptor.getUnit();
    if(unit.assigned()) {
        int64_t unitId = unit.getId();
        std::string unitName = unit.getName();
        std::string unitQuantity = unit.getQuantity();
        std::string unitSymbol = unit.getSymbol();
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["UnitId"] = unitId;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["Description"] = unitName;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["Quantity"] = unitQuantity;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["DisplayName"] = unitSymbol;
    }
    else
        parentJsonValue["Value"]["DataDescriptor"]["Unit"] = Json::ValueType::nullValue;
    ScalingPtr postScaling = dataDescriptor.getPostScaling();
    if(postScaling.assigned()) { 
        SampleType postScalingInputSampleType = postScaling.getInputSampleType();;
        ScaledSampleType postScalingOutputSampleType = postScaling.getOutputSampleType();
        parentJsonValue["Value"]["DataDescriptor"]["PostScaling"]["InputSampleType"] = int(postScalingInputSampleType);
        parentJsonValue["Value"]["DataDescriptor"]["PostScaling"]["OutputSampleType"] = int(postScalingOutputSampleType);
    }
    else
        parentJsonValue["Value"]["DataDescriptor"]["PostScaling"] = Json::ValueType::nullValue;
    StringPtr origin = dataDescriptor.getOrigin();
    if(origin.assigned())
        parentJsonValue["Value"]["DataDescriptor"]["Origin"] = toStdString(origin);
    else
        parentJsonValue["Value"]["DataDescriptor"]["Origin"] = Json::ValueType::nullValue;
    RatioPtr tickResolution = dataDescriptor.getTickResolution();
    if(tickResolution.assigned()) {
        int64_t numerator = tickResolution.getNumerator();
        int64_t denominator = tickResolution.getDenominator();
        parentJsonValue["Value"]["DataDescriptor"]["TickResolution"]["Numerator"] = numerator;
        parentJsonValue["Value"]["DataDescriptor"]["TickResolution"]["Denominator"] = denominator;
    }
    else
        parentJsonValue["Value"]["DataDescriptor"]["TickResolution"] = Json::ValueType::nullValue;
    RangePtr valueRange = dataDescriptor.getValueRange();
    if(valueRange.assigned()) {
        double lowValue = valueRange.getLowValue();
        double highValue = valueRange.getHighValue();
        parentJsonValue["Value"]["DataDescriptor"]["ValueRange"]["Low"] = lowValue;
        parentJsonValue["Value"]["DataDescriptor"]["ValueRange"]["High"] = highValue;
    }
    else
        parentJsonValue["Value"]["DataDescriptor"]["ValueRange"] = Json::ValueType::nullValue;
}

END_NAMESPACE_JET_MODULE