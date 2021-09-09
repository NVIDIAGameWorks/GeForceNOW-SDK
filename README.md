# NVIDIA GeForce NOW SDK Release 1.7

## At a Glance

The GeForce NOW SDK (GFN SDK) is a means for game developers and publishers to directly integrate with GeForce NOW, NVIDIA's Cloud Gaming Service. This service allows gamers to experience GeForce gaming anywhere, as well as allowing publishers and game developers to take advantage of high-performance rendering through NVIDIA's top-notch DirectX and Vulkan drivers on both Windows and Linux platforms.

The GFN SDK is ever-evolving to provide easy integration of GeForce NOW features into publisher applications and games, as well as more efficient way to integrate games into the GeForce NOW ecosystem.

Please refer to the [SDK GFN Primer](./doc/SDK-GFN-PRIMER.pdf) for a more detailed overview of the features.

### What's New in This Release

* Adds version 1 of GfnGetClientInfo API
  * Provides Operating System information about the user's client system
  * Collapses several other GetClient APIs into this single API
  * Provides callback support when the client changes during a streaming session (Example: user starts session on PC, then resumes on a mobile device)
  * Future SDK releases will include additional client information
* Fix for potential crash after GfnShutdownSDK API is called
* Fix for SDK failure when SDK is initialized more than once in a single process
* Fix in GfnStartStream if the GFN streamer fails to stop a previous streaming session gracefully
* Various API optimizations

## Developer Content Portal

* If your organization or game isn't yet registered with NVIDIA, visit the [Developer Content Portal](https://portal-developer.nvidia.com/) to create accounts and complete game registration.

## Development Guide

### Software Stack

![Software Stack](./doc/img/software_stack.png)

Each feature has different integration points. Some features define REST Web APIs, while others integrate into the game or publisher application with native C interfaces. Refer to documentation for each feature in the doc folder.

Some features require a compatible version of GeForce NOW to be installed on the client system. The integrated GFN SDK components are designed to take care of downloading and installing GeForce NOW client when needed.

### GeForce NOW SDK Package

The distribution is laid out as below:
```
.
├─── CMakeLists.txt
├─── generate.bat
├─── generate_x86.bat
├─── LICENSE
├─── LICENSE.samplelauncher.thirdparty
├─── LICENSE.thirdparty
├─── README.md
|
├─── doc
|   │   SDK-GFN-PRIMER.pdf
|   │   SDK-GFN-NGN-ENDPOINT.pdf
|   │   SDK-GFN-ACCOUNT-LINKING-SSO-GUIDE.pdf
|   │   SDK-NVIDIA-IDENTITY-FEDERATION-SYSTEM.pdf
|   └───SDK-GFN-RUNTIME
|           index.html
|
├───include
│       GfnRuntimeSdk_CAPI.h
│       GfnRuntimeSdk_Wrapper.c
│       GfnRuntimeSdk_Wrapper.h
│       GfnSdk.h
│       GfnSdk_SecureLoadLibrary.c
│       GfnSdk_SecureLoadLibrary.h
│
├───lib
│   ├───x64
│   │       GfnRuntimeSdk.dll
│   │
│   └───x86
│           GfnRuntimeSdk.dll
│
└───samples
    |   README.md
    ├───SampleCApp
    └───SampleLauncher

```

### Identity management

* For Account and IDM-related APIs, please refer to the document /doc/SDK-NVIDIA-IDENTITY-FEDERATION-SYSTEM.pdf

### Additional Documentation Online

In addition to the documents included in /doc in this repository, there are online documentation resources for some of the features.
* For game catalog APIs, please refer to the [GFN Application Catalog Query API Help](https://games.geforce.com/help/).
