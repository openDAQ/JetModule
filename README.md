# JetModule

Jet module is an integration of Jet protocol with openDAQ SDK. It publishes device structure as Jet states.

## How to use

- Fetch JetModule into your project and link it to your device's executable.
- Include `<jet_server.h>` header in your source file.
- After a device has been instantiated, create `JetServer` object:
  
  ```c++
  daq::modules::jet_module::JetServer jetServer = JetServer(device);
  ```

- Call `JetServer::publishJetStates()` to publish device structure as Jet states:

  ```c++
  jetServer.publishJetStates();
  ```

Jet states are updated automatically if some property value is changed.

### CMake options

`COMPILE_REFERENCE_APPLICATION` - Compiles reference application when ON.

## Build

```bash
mkdir build
cd build
cmake ../
cmake --build .
```

### TODO

- Add Support for all property types.
