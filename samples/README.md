This directory contains sample projects that demonstrate proper use of the various APIs that are part of **GeForce NOW (GFN) SDK**.
Each sample is designed to provide examples of API usage in the most comment integration scenarios.

## Sample Overview

### CGameAPISample
This C-based simple command-line sample demonstrates usage of the game-focused APIs to detect the GeForce NOW cloud environment and control behavior of a game in that environment. This sample focuses on use of callbacks to notify a title of GeForce NOW cloud environment state changes.

### CloudCheckAPI
This C-based simple command-line sample demonstrates usage of the APIs dedicated to checking if running in the GFN cloud environment. It is designed to be run in both client and cloud environments to provide expected results in each of the environments.

### CubeSample
This is a modified variant of Vulkan Cube app originally distributed with Vulkan SDK. It demonstrates integration with GFN SDK as well as some user controls that use two-way communication.
See the sample [README](./CubeSample/README.md) for more details.

### OpenClientBrowser
This C-based simple command-line sample demonstrates usage of the the GfnOpenURLOnClient API. Opening URLs on the connecting client's browser.

### PartnerDataAPI
This C-based simple command-line sample demonstrates usage of the two APIs dedicated to obtaining partner-supplied data provided during session initialization, as well as the correct way to free the memory allocated for the data.

### PreWarmSample
This C-based simple command-line sample demonstrates usage of the APIs and callbacks associated with putting an application into PreWarm state, and waiting for a user session to connect. This allows an application to preload all common data, allowing a streaming user to bypass must of the loading time and get to the main menu much faster.

## Building the Samples

Please see the [SDK root README](../README.md) for details on how to build the samples.