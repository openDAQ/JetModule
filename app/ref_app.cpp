#include <opendaq/opendaq.h>
#include <jet_server.h>
#include <iostream>

using namespace daq;
using namespace daq::modules::jet_module;

int main() {

    // Create an openDAQ instance, loading modules at MODULE_PATH
    const InstancePtr instance = Instance(MODULE_PATH);

    // Add a reference device as root device
    instance.setRootDevice("daqref://device0");

    auto device = instance.getRootDevice();
    device.addDevice("daqref://device1");

    // Add function blocks for testing purposes
    // instance.addFunctionBlock("ref_fb_module_renderer");
    instance.addFunctionBlock("ref_fb_module_statistics");
    instance.addFunctionBlock("ref_fb_module_power");
    instance.addFunctionBlock("ref_fb_module_scaling");
    device.addFunctionBlock("ref_fb_module_classifier");
    device.addFunctionBlock("ref_fb_module_trigger");

    // Start streaming openDAQ OpcUa server
    instance.addServer("openDAQ OpcUa", nullptr);

    JetServer myJet = JetServer(instance);
    myJet.publishJetStates();


    std::cout << "Press \"enter\" to exit the application..." << std::endl;
    std::cin.get();

    return 0;
}