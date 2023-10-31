# JetModule
Jet module is an integration of Jet protocol with openDAQ SDK. It publishes device structure as Jet states.\
JetModule has to be used within a device module. Using it on application side will not provide ability to get updating Jet states because openDAQ SDK does not provide ability to do that currently.

## How to use
- Fetch JetModule into your project and link it to your device module.
- Create `JetServer` in your device module.
- Call `JetServer::publishJetStates()` to publish initial device structure as Jet states.
- Call `JetServer::updateJetState(const ComponentPtr& component)` in every property's callback function to ensure the update of a Jet state once some property value changes.

### CMake options
`COMPILE_REFERENCE_APPLICATION` - Compiles reference application when ON.

## Build
```
mkdir build
cd build
cmake ../
cmake --build .
```



### TODO
- Add Support for all property types.