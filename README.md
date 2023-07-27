# NVIDIA GeForce NOW SDK Release 2.0

## At a Glance

The GeForce NOW SDK (GFN SDK) is a means for game developers and publishers to directly integrate with GeForce NOW, NVIDIA's Cloud Gaming Service. This service allows gamers to experience GeForce gaming anywhere, as well as allowing publishers and game developers to take advantage of high-performance rendering through NVIDIA's top-notch DirectX and Vulkan drivers on both Windows and Linux platforms.

The GFN SDK is ever-evolving to provide easy integration of GeForce NOW features into publisher applications and games, as well as more efficient way to integrate games into the GeForce NOW ecosystem.

Please refer to the [SDK GFN Primer](./doc/SDK-GFN-PRIMER.pdf) for a more detailed overview of the features.

### What's New in This Release

* Deprecates user authentication flow through third party applications; StartStream API no longer accepts NVIDIA IDM Token string (param `pchAuthToken`) or Token identifier (param `tokenType`)
* Adds support for Visual Studio 2022
* Bug fixes and minor improvements

## GeForce NOW Developer Portal

* Please [contact us](mailto:gdp-queries@nvidia.com) to register your organization or game with the GeForce NOW Developer Portal.

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
|   │   SDK-GFN-ACCOUNT-LINKING-SSO-GUIDE.pdf
|   │   SDK-GFN-CLOUD-API.pdf
|   │   SDK-GFN-DEEP-LINKING.pdf
|   │   SDK-GFN-MOBILE-TOUCH-INTEGRATION-GUIDE.pdf
|   │   SDK-GFN-NGN-ENDPOINT.pdf
|   │   SDK-GFN-PRIMER.pdf
|   └───SDK-GFN-RUNTIME
|       └───index.html
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
    ├───CGameAPISample
    ├───SampleLauncher
    └───SDKDllDirectRefSample

```
