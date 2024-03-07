#include "device_converter.h"

BEGIN_NAMESPACE_JET_MODULE

/**
 * @brief Composes Json representation of an openDAQ device and publishes it as Jet state.
 * This function is overriden by every Converter class in order to convert different openDAQ objects according to the data they host.
 * 
 * @param component OpenDAQ device which has to be converted into its Json representation.
 */
void DeviceConverter::composeJetState(const ComponentPtr& component)
{
    Json::Value jetState;

    // Parsing the device to identify its properties
    appendProperties(component, jetState);   

    // Adding additional information to a device's Jet state
    appendObjectType(component, jetState);
    appendActiveStatus(component, jetState);
    appendVisibleStatus(component, jetState);
    appendTags(component, jetState);    

    appendDeviceMetadata(component.asPtr<IDevice>(), jetState);
    appendDeviceDomain(component.asPtr<IDevice>(), jetState);

    // Creating callbacks
    createOpendaqCallback(component);
    JetStateCallback jetStateCallback = createJetCallback();

    // Publish the component's tree structure as a Jet state
    std::string path = component.getGlobalId();
    jetPeerWrapper.publishJetState(path, jetState, jetStateCallback);
}

/**
 * @brief Appends device metadata information to a Json object which is published as a Jet state. 
 * 
 * @param device Device from which metadata is retrieved.
 * @param parentJsonValue Json object to which metadata is appended.
 */
void DeviceConverter::appendDeviceMetadata(const DevicePtr& device, Json::Value& parentJsonValue)
{
    auto deviceInfo = device.getInfo();
    auto deviceInfoProperties = deviceInfo.getAllProperties();
    for(auto property : deviceInfoProperties) 
    {
        propertyManager.determinePropertyType<DeviceInfoPtr>(deviceInfo, property, parentJsonValue);
    }
}

/**
 * @brief Appends device domain data (e.g. time domain information) to a Json object which is published as a Jet state. 
 * 
 * @param device Device from which domain data is retrieved.
 * @param parentJsonValue Json object to which domain data is appended.
 */
void DeviceConverter::appendDeviceDomain(const DevicePtr& device, Json::Value& parentJsonValue)
{
    DeviceDomainPtr domain = device.getDomain();
    RatioPtr tickResolution = domain.getTickResolution();
        int64_t numerator = tickResolution.getNumerator();
        int64_t denominator = tickResolution.getDenominator();
        parentJsonValue["Domain"]["Resolution"]["Numerator"] = numerator;
        parentJsonValue["Domain"]["Resolution"]["Denominator"] = denominator;
    uint64_t ticksSinceResolution = domain.getTicksSinceOrigin();
        parentJsonValue["Domain"]["TicksSinceOrigin"] = ticksSinceResolution;
    std::string origin = domain.getOrigin();
        parentJsonValue["Domain"]["Origin"] = origin;
    UnitPtr unit = domain.getUnit();
        int64_t id = unit.getId();
        std::string name = unit.getName();
        std::string quantity = unit.getQuantity();
        std::string symbol = unit.getSymbol();
        parentJsonValue["Domain"]["Unit"]["UnitId"] = id;
        parentJsonValue["Domain"]["Unit"]["Description"] = name;
        parentJsonValue["Domain"]["Unit"]["Quantity"] = quantity;
        parentJsonValue["Domain"]["Unit"]["DisplayName"] = symbol;
}

END_NAMESPACE_JET_MODULE