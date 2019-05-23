# NVIDIA GeForce NOW SDK #

## At a Glance

The GeForce NOW SDK (GFNSDK) is a means for game developers and publishers to directly integrate GeForce NOW, NVIDIA's Cloud Gaming Service. This service allows gamers to experience GeForce gaming anywhere - PC, Mac or NVIDIA Shield TV, as well as allowing publishers and game developers to take advantage of high-performance rendering through NVIDIA's top-notch DirectX and Vulkan drivers on both Windows and Linux platforms. 

The GFN SDK is ever-evolving to provide easy integration of GeForce NOW features into publisher applications and games, as well as more efficient way to integrate games into the GeForce NOW ecosystem. 

At this time, this repository provides a sneak-peak into the APIs for Stream and Single Sign-on integration into your launcher application, as well as information on the APIs for providing new builds of games into the GeForce NOW environment. Full APIs for these as well as new features will be made available in the near future.

Please refer to the [GFN SDK Primer](./doc/GFN_SDK_Primer.pdf) for a more detailed overview of the features.

## Developer Content Portal

* If your organization or game isn't yet registered with NVIDIA, visit the [Developer Content Portal](https://portal-developer.nvidia.com/) to create accounts and complete game registration.

## Development Guide

### Software Stack

![Software Stack](./doc/img/software_stack.png)

Each feature has different integration points. Some features define REST Web APIs, while others integrate into the game or publisher application with native C interfaces. Refer to documentation for each feature in the doc folder.

Some features require a compatible version of GeForce NOW to be installed on the client system. The integrated GFNSDK components are designed to take care of downloading and installing GeForce NOW client when needed.

### GfeSDK Package

The distribution is laid out as below:
```
.
+-- README.md
+-- LICENSE
+-- doc
|   GFN_SDK_Primer.pdf
|   GFN_SDK_Runtime_API.pdf
|   SDK-GFN-NGN-ENDPOINT.pdf
|   SDK-GFN-SUPPORTED-TITLES.pdf
+-- include
|   GfnRuntimeSdk_CAPI.h
```

### Additional Documentation Online

In addition to the documents included in /doc in this repository, there are online resources for some of the features. Please refer to the [NVIDIA Developer Services Help](https://portal-developer.nvidia.com/help/).


