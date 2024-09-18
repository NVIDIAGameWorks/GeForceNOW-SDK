This directory contains sample projects that demonstrate proper use of the various APIs that are part of **GeForce NOW (GFN) SDK**.
Each sample is designed to provide examples of API usage in the most comment integration scenarios.

## Sample Overview

### CGameAPISample
This C-based simple command-line sample demonstrates usage of the game-focused APIs to detect the GeForce NOW cloud environment and control behavior of a game in that environment. This sample focuses on use of callbacks to notify a title of GeForce NOW cloud environment state changes.

### CloudCheckAPI
This C-based simple command-line sample demonstrates usage of the APIs dedicated to checking if running in the GFN cloud environment. It is designed to be run in both client and cloud environments to provide expected results in each of the environments.

### GdnSampleApp
This is a modified variant of Vulkan Cube app originally distributed with Vulkan SDK. It demonstrates integration with GFN SDK as well as some user controls that use two-way communication.
See the sample [README](./GdnSampleApp/README.md) for more details.

### PartnerDataAPI
This C-based simple command-line sample demonstrates usage of the two APIs dedicated to obtaining partner-supplied data provided during session initialization, as well as the correct way to free the memory allocated for the data.

### PreWarmSample
This C-based simple command-line sample demonstrates usage of the APIs and callbacks associated with putting an application into PreWarm state, and waiting for a user session to connect. This allows an application to preload all common data, allowing a streaming user to bypass must of the loading time and get to the main menu much faster.

### OpenClientBrowser
This C-based simple command-line sample demonstrates usage of the the GfnOpenURLOnClient API. Opening URLs on the connecting client's browser.


### SampleLauncher
This C++-based sample demonstrates usage of the Launcher/Publisher application-focused APIs, including getting a list of supported titles supported by GeForce NOW, as well as invoking the GeForce NOW Windows client to start a streaming session of a title. This sample is meant to be run on both the local client and GeForce NOW cloud environment to understand how all the APIs behave in each environment.
   
In addition to supporting both execution environments, this sample is built on Chromium Embedded Framework (CEF) to provide a view of how a CEF-based application can make use of the API. The sample source is a fork of the CEF project's example source found at https://bitbucket.org/chromiumembedded/cef-project/src.

When building for Linux, this sample requires the X11-dev libraries be installed to compile successfully.
    
For applications that are not CEF-based, users can focus on API calls as found in [SampleLauncher's gfn_sdk_helper.cc file](./SampleLauncher/src/gfn_sdk_demo/gfn_sdk_helper.cc).

### SDKDllDirectRefSample

This C-based sample demonstrates basic SDK usage without relying on the wrapper helper functions. This can be useful for partners who are unable to utilize the wrapper in their build environment or want finer control on SDK library loading and how the library exports are called from an application.

## Building the Samples

Please see the [root README](./README.md) for details on how to build the samples.