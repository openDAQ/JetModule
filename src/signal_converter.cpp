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
        parentJsonValue["Value"]["DataDescriptor"]["Rule"] = std::string(rule);
    SampleType sampleType;
        // SampleType::Invalid results in runtime error when we try to get the sample type
        // Due to this, getter function is inside of try-catch block
        try {
            sampleType = dataDescriptor.getSampleType();
        } catch(...) {
            sampleType = SampleType::Invalid;
        }
        parentJsonValue["Value"]["DataDescriptor"]["SampleType"] = int(sampleType);
    UnitPtr unit = dataDescriptor.getUnit();
        int64_t unitId = unit.getId();
        std::string unitName = unit.getName();
        std::string unitQuantity = unit.getQuantity();
        std::string unitSymbol = unit.getSymbol();
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["UnitId"] = unitId;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["Description"] = unitName;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["Quantity"] = unitQuantity;
        parentJsonValue["Value"]["DataDescriptor"]["Unit"]["DisplayName"] = unitSymbol;
    ScalingPtr postScaling = dataDescriptor.getPostScaling();
        // SampleType::Invalid and ScaledSampleType::Invalid result in runtime error when we try to get them
        // Due to thism getter functions are inside of try-catch blocks
        SampleType postScalingInputSampleType;
        ScaledSampleType postScalingOutputSampleType;
        try {
            postScalingInputSampleType = postScaling.getInputSampleType();
        } catch(...) {
            postScalingInputSampleType = SampleType::Invalid;
        }
        try {
            postScalingOutputSampleType = postScaling.getOutputSampleType();
        } catch(...) {
            postScalingOutputSampleType = ScaledSampleType::Invalid;
        }
        parentJsonValue["Value"]["DataDescriptor"]["PostScaling"]["InputSampleType"] = int(postScalingInputSampleType);
        parentJsonValue["Value"]["DataDescriptor"]["PostScaling"]["OutputSampleType"] = int(postScalingOutputSampleType);
    StringPtr origin = dataDescriptor.getOrigin();
        parentJsonValue["Value"]["DataDescriptor"]["Origin"] = toStdString(origin);
    RatioPtr tickResolution = dataDescriptor.getTickResolution();
        // If the tick resolution is not applicable to the signal, getter functions throw runtime errors
        // Due to this, we have to use try-catch block and assign 0s as default values in that case
        int64_t numerator;
        int64_t denominator;
        try {
            numerator = tickResolution.getNumerator();
            denominator = tickResolution.getDenominator();
        } catch(...) {
            numerator = 0;
            denominator = 0;
        }
        parentJsonValue["Value"]["DataDescriptor"]["TickResolution"]["Numerator"] = numerator;
        parentJsonValue["Value"]["DataDescriptor"]["TickResolution"]["Denominator"] = denominator;
    RangePtr valueRange = dataDescriptor.getValueRange();
        // If the value range is not applicable to the signal, getter functions throw runtime errors
        // Due to this, we have to use try-catch block and assign 0s as default values in that case
        double lowValue;
        double highValue;
        try {
            lowValue = valueRange.getLowValue();
            highValue = valueRange.getHighValue();
        } catch(...) {
            lowValue = 0;
            highValue = 0;
        }
        parentJsonValue["Value"]["DataDescriptor"]["ValueRange"]["Low"] = lowValue;
        parentJsonValue["Value"]["DataDescriptor"]["ValueRange"]["High"] = highValue;
}

END_NAMESPACE_JET_MODULE