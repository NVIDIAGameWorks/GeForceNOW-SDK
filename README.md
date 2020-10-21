# NVIDIA GeForce NOW SDK Release 1.4

## At a Glance

The GeForce NOW SDK (GFNSDK) is a means for game developers and publishers to directly integrate with GeForce NOW, NVIDIA's Cloud Gaming Service. This service allows gamers to experience GeForce gaming anywhere, as well as allowing publishers and game developers to take advantage of high-performance rendering through NVIDIA's top-notch DirectX and Vulkan drivers on both Windows and Linux platforms. 

The GFN SDK is ever-evolving to provide easy integration of GeForce NOW features into publisher applications and games, as well as more efficient way to integrate games into the GeForce NOW ecosystem. 

Please refer to the [GFN SDK Primer](./doc/GFN-SDK-PRIMER.pdf) for a more detailed overview of the features.

### What's New in This Release
* New NVIDIA Identity Federation System
* gfnRequestGfnAccessToken, gfnGetTitlesAvailableRelease APIs are deprecated
* All get APIs (gfnGetClientIp, gfnGetClientLanguageCode, gfnGetCustomData) will allocate buffers in dynamic memory instead of using static memory
* New gfnFree API that releases memory allocated by Get functions
* New getAuthData API to retrieve custom authorized data
* Fixed filename typos (GfnRuntimeSdk_SecureLoadLibrary.*)
* Fixed a variety of minor bugs


## Developer Content Portal

* If your organization or game isn't yet registered with NVIDIA, visit the [Developer Content Portal](https://portal-developer.nvidia.com/) to create accounts and complete game registration.

## Development Guide

### Software Stack

![Software Stack](./doc/img/software_stack.png)

Each feature has different integration points. Some features define REST Web APIs, while others integrate into the game or publisher application with native C interfaces. Refer to documentation for each feature in the doc folder.

Some features require a compatible version of GeForce NOW to be installed on the client system. The integrated GFNSDK components are designed to take care of downloading and installing GeForce NOW client when needed.

### GeForce NOW SDK Package

The distribution is laid out as below:
```
.
+-- CMakeLists.txt
+-- generate.bat
+-- generate_x86.bat
+-- LICENSE
+-- README.md
+-- doc
|       GFN-SDK-RUNTIME
|       index.html
|       GFN-SDK-PRIMER.pdf
|       SDK-GFN-NGN-ENDPOINT.pdf
|       SDK-NVIDIA-IDENTITY-FEDERATION-SYSTEM.pdf
+-- include
|       GfnRuntimeSdk_CAPI.h
|       GfnRuntimeSdk_SecureLoadLibrary.c
|       GfnRuntimeSdk_SecureLoadLibrary.h
|       GfnRuntimeSdk_Wrapper.c
|       GfnRuntimeSdk_Wrapper.h
+-- lib
+----- x64
|       GfnRuntimeSdk.dll
+----- x86
|       GfnRuntimeSdk.dll
+-- samples
|       SampleCApp
|       SampleLauncher
|       README.md
```
### Identity management

* For Account and IDM-related APIs, please refer to the document /doc/SDK-NVIDIA-IDENTITY-FEDERATION-SYSTEM.pdf

### Additional Documentation Online

In addition to the documents included in /doc in this repository, there are online documentation resources for some of the features.
* For game and launcher related APIs, please refer to the [NVIDIA Developer Services API Help](https://portal-developer.nvidia.com/help/).
