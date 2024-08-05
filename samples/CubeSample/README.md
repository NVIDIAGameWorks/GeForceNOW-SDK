# GFN SDK CUBE SAMPLE

*GFN SDK Cube Sample* (CubeSample) is a sample application distributed with the GeForce NOW SDK.
This application is a modified variant of vkcube application distributed by [LunarG](https://www.lunarg.com/) with the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (version `1.3.280.0`) under Apache 2.0 License.

This sample specializes in demonstrating two-way communication between app and the client, which can be particularly useful in [Graphics Delivery Network (GDN)](https://www.nvidia.com/en-in/omniverse/solutions/stream-3d-apps/). GDN, part of NVIDIA Omniverse™ Cloud, takes advantage of NVIDIA’s global cloud streaming infrastructure to deliver seamless access to high-fidelity 3D interactive experiences. It focuses on non-Gaming graphical applications like Product Configurators, Simulators and other 3D interactive experiences. With the help of the *GFN Client SDK* ,  GDN content can also be streamed directly into your website browser. Cube Sample app is a graphical sample demonstrating core integrations with GFN SDK like cloud check and two-way messaging.

Two way messaging is meaningfully demonstrated by utilizing simple user controls (Keyboard/Mouse) in the application. User input (keyboard and mouse) to the app controls the cube spin, and the app sends the updated spin rate to the client (Website) using the `GfnSendMessage` API which meaningfully illustrates this change in spin state. Likewise, the website can send messages to the Application running on the server using corresponsing `GFN Client SDK` API. 
Refer to the below list of user control and corresponding messaging.

| ACTION | APP CONTROLS | COMMAND FROM CLIENT TO APP | ACKNOWLEDGEMENT FROM APP TO CLIENT |
| -------- | ------- | ------- | ------- |
| SPIN FASTER   | Right Arrow Key   | "spin+"       | "spin N"  |
| SPIN SLOWER   | Left Arrow Key    | "spin-"       | "spin N" |
| REVERSE SPIN  | Up/Down Arrow Key | "respin"      | "spin N" |
| PAUSE/UNPAUSE | Space Bar         | "togglePause" | "spin N" |

Note: the spin value "N" is a floating point number (2 decimal places, i.e. `%0.2f` format) corresponding to current change in angle (in degrees) per frame.

Optionally there are also a few mouse controls:
| ACTION | APP CONTROLS |
| -------- | ------- |
| SPIN FASTER   | Left Click on right-half of Screen |
| SPIN SLOWER   | Left Click on left-half of Screen  |
| REVERSE SPIN  | Right Click anywhere on Screen     |

## CubeSample Repository Layout

```
.
│   CMakeLists.txt    - makefile
│   GeForceNOW.ico    - app icon
│   GfnSdkInterface.c - GFN SDK interface code for the application
│   GfnSdkInterface.h - GFN SDK interface code for the application
│   Main.c            - main entrypoint
│   README.md         - this file
│   resource.h        - app resource
│   resource.rc       - app resource
│
└───cube
    │   cube.c        - Core implementation of vkcube application
    │   cube.h        - Declerations that are required outside of core application
    │
    └───Include
        │   cube.frag.inc - Fragment shader bytecode in the SPIR-V format
        │   cube.vert.inc - Vertex shader bytecode in the SPIR-V format
        │   gettime.h     - time utilities
        │   linmath.h     - math utilities
        │   nvidia_logo.h - PPM encoded texture of NVIDIA logo
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