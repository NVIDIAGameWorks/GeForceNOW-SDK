# NVIDIA GeForce NOW SDK Release 1.5

## At a Glance

The GeForce NOW SDK (GFN SDK) is a means for game developers and publishers to directly integrate with GeForce NOW, NVIDIA's Cloud Gaming Service. This service allows gamers to experience GeForce gaming anywhere, as well as allowing publishers and game developers to take advantage of high-performance rendering through NVIDIA's top-notch DirectX and Vulkan drivers on both Windows and Linux platforms. 

The GFN SDK is ever-evolving to provide easy integration of GeForce NOW features into publisher applications and games, as well as more efficient way to integrate games into the GeForce NOW ecosystem. 

Please refer to the [GFN SDK Primer](./doc/SDK-GFN-PRIMER.pdf) for a more detailed overview of the features.

### What's New in This Release
* New gfnIsRunningInCloudSecure API for tamper-resistant queries of GFN environment. See function documentation on requirements to use this API.
* New APIs to allow your game to function in a pre-loaded, ready for user connection model, allowing users to get into their gaming session faster.
    * The gfnRegisterSessionInit API and corresponding callback to allow your game to be notified when a GFN user has initiated a session with your game.
    * New gfnAppReady API to signal your game has finished loading and is ready for frame streaming to begin. This allows game loading screens to be skipped from streaming to the user to improve the user experience while playing your game.
* Changed GFN Application Catalog Query API to new GraphQL-based LCARS-based REST endpoint. See Sample Launcher source for example usage.
* Refactor and renaming of common objects and definitions to a more generic "GfnSdk" naming convention. This makes the SDK more extensible and reusable for new API features in upcoming releases. 
* Refactor of execution flow of cloud-specific APIs such that any application that makes API calls only in the GFN cloud environment does not need to package the client SDK library with their application.
* Several minor bug and crash fixes.

For more information on moving your game to using the gfnIsRunningInCloudSecure API, or moving your game to the pre-loading model described above, contact your NVIDIA Developer representative.

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
+-- CMakeLists.txt
+-- generate.bat
+-- generate_x86.bat
+-- LICENSE
+-- LICENSE.samplelauncher.thirdparty
+-- LICENSE.thirdparty
+-- README.md
+-- doc
|       GFN-SDK-RUNTIME
|       index.html
|       SDK-GFN-PRIMER.pdf
|       SDK-GFN-NGN-ENDPOINT.pdf
|       SDK-NVIDIA-IDENTITY-FEDERATION-SYSTEM.pdf
+-- include
|       GfnRuntimeSdk_CAPI.h
|       GfnRuntimeSdk_Wrapper.c
|       GfnRuntimeSdk_Wrapper.h
|       GfnSdk.h
|       GfnSdk_SecureLoadLibrary.c
|       GfnSdk_SecureLoadLibrary.h
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
* For game catalog APIs, please refer to the [GFN Application Catalog Query API Help](https://games.geforce.com/help/).
