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
    
    // Start streaming and openDAQ OpcUa servers
    // instance.addStandardServers();

    auto device = instance.getRootDevice();

    JetServer myJet = JetServer(device);

    myJet.printSomething();
    
    std::cout << "Press \"enter\" to exit the application..." << std::endl;
    std::cin.get();

    return 0;
}