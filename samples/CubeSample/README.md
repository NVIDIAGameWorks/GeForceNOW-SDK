# GFN SDK CUBE SAMPLE

## Overview
*GFN SDK Cube Sample* (CubeSample) is a sample application distributed with the GeForce NOW SDK.
This application is a modified variant of vkcube application distributed by [LunarG](https://www.lunarg.com/) with the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (version `1.3.280.0`) under Apache 2.0 License.

This graphical sample specializes in demonstrating two-way communication between the application running in the GeForce NOW cloud environment and the client, which can be particularly useful in [Graphics Delivery Network (GDN)](https://www.nvidia.com/en-in/omniverse/solutions/stream-3d-apps/), part of NVIDIA Omniverse™ Cloud. GDN takes advantage of NVIDIA’s global cloud streaming infrastructure to deliver seamless access to high-fidelity 3D interactive experiences. It focuses on non-Gaming graphical applications like Product Configurators, Simulators and other 3D interactive experiences. With the help of the *GFN Client SDK*, GDN content can also be streamed directly into your website browser. For more information in GDN and the *GFN Client SDK*, refer to the documentation available on the [GeForce NOW Developer Portal](https://developer.geforcenow.com/)

Two-way messaging is demonstrated by utilizing simple user controls (Keyboard/Mouse) in this sample application. User input (keyboard and mouse) to the app controls the cube spin, and the app sends the updated spin rate to the client (Website) using the `GfnSendMessage` API which illustrates this change in spin state. Likewise, the client application or website can send messages to the application running on the server using corresponsing `GFN Client SDK` API, which are received via the `MessageCallback` callback function registerd via the `GfnRegisterMessageCallback` API.

## Intended Audience
This sample is intended for game and launcher developers that need their games and applications communicating with each other across GeForce NOW without having to create a custom two-way communication stack.

## APIs Demonstrated
GfnIsRunningInCloud
GfnSendMessage
GfnRegisterMessageCallback (registers MessageCallback)

All GFNSDK API calls are localized to GfnSdkInterface.c to make it easier to review the code demonstrating these APIs.

## CubeSample Repository Layout

```
.
│   CMakeLists.txt    - makefile
│   GeForceNOW.ico    - app icon
│   GfnSdkInterface.c - source that calls GFN SDK APIs 
│   GfnSdkInterface.h - header that defines functions that call GFN SDK APIs
│   Main.c            - main entrypoint source
│   README.md         - this file
│   resource.h        - app resource
│   resource.rc       - app resource
│
└───cube
    │   cube.c        - Core implementation of vkcube application
    │   cube.h        - Declarations that are required outside of core application
    │
    └───Include
        │   cube.frag.inc - Fragment shader bytecode in the SPIR-V format
        │   cube.vert.inc - Vertex shader bytecode in the SPIR-V format
        │   gettime.h     - time utilities
        │   linmath.h     - math utilities
        │   nvidia_logo.h - PPM encoded texture of NVIDIA logo used on the rendered cube
        │   object_type_string_helper.h - enum to string map definition
        │
        ├───Volk
        │       <Volk source code: Meta loader for Vulkan API>
        │
        ├───vk_video
        │       <Vulkan SDK video codec headers>
        │
        └───vulkan
                <required subset of Vulkan SDK headers>
```

## User Controls
Refer to the following table of user controls and corresponding messaging sent through the GFN SDK APIs.

| ACTION | APP CONTROLS | COMMAND FROM CLIENT TO APP | ACKNOWLEDGEMENT FROM APP TO CLIENT |
| -------- | ------- | ------- | ------- |
| SPIN FASTER   | Right Arrow Key   | "spin+"       | "spin <N>"  |
| SPIN SLOWER   | Left Arrow Key    | "spin-"       | "spin <N>" |
| REVERSE SPIN  | Up/Down Arrow Key | "respin"      | "spin <N>" |
| PAUSE/UNPAUSE | Space Bar         | "togglePause" | "spin <N>" |

Note: the spin value "<N>" is a floating point number (2 decimal places, i.e. `%0.2f` format) corresponding to current change in angle (in degrees) per frame.

Optionally there are also a few mouse controls:
| ACTION | APP CONTROLS |
| -------- | ------- |
| SPIN FASTER   | Left Click on right-half of Screen |
| SPIN SLOWER   | Left Click on left-half of Screen  |
| REVERSE SPIN  | Right Click anywhere on Screen     |
