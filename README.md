# JetModule

Jet module is an integration of Jet protocol with openDAQ SDK. It publishes device structure as Jet states.

## How to use

- Fetch JetModule into your project and link it to your device's executable.
- Include `<jet_server.h>` header in your source file.
- After a device has been instantiated, create `JetServer` object:
  
  ```c++
  jet_module::JetServer jetServer = jet_module::JetServer(opendaqInstance);
  ```

- Call `JetServer::publishJetStates()` to publish device structure as Jet states:

  ```c++
  jetServer.publishJetStates();
  ```

Jet states are updated automatically if some property value is changed.

### CMake options

`COMPILE_REFERENCE_APPLICATION` - Compiles reference application when ON.\
`JET_MODULE_ENABLE_TESTS` - Compiles tests if enabled.\
`IGNORE_INSTALLED_SDK` - Ignores loccally installed SDK and fetches it if enabled.

## Build

```bash
mkdir build
cd build
cmake ../
cmake --build .
```

### TODO

- Add Support for all property types.
- ref_dev0/MethodSet/GetErrorInformation (Decide whether methods like these habe to be turned into Jet methods).
- Verify that Jet methods for FunctionProperty is created properly.
- Support parameter passing to Jet methods
- Rethink how to handle SelectionProperties
