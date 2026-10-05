# Overview
This directory contains sample projects that demonstrate proper use of the various APIs that are part of GeForce NOW (GFN) SDK.
Each sample is designed to provide examples of API usage in the most common integration scenarios.

# Samples

## CGameAPISample
This C-based simple command-line sample demonstrates usage of the game-focused APIs to detect the GeForce NOW cloud environment and control behavior of a game in that environment.
This sample focuses on use of callbacks to notify a title of GeForce NOW cloud environment state changes.

### Intended Audience
The sample is intended for game developers that want their games to know when running in GeForce NOW and be able to send and receive data to and from GeForce NOW services.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnCloudCheck with GfnCloudCheckChallenge and GfnCloudCheckResponse usage
- GfnRegisterExitCallback
- GfnRegisterPauseCallback
- GfnRegisterSaveCallback
- GfnRegisterSessionInitCallback
- GfnRegisterMessageCallback
- GfnRegisterClientInfoCallback
- GfnRegisterNetworkStatusCallback
- GfnGetClientIpV4
- GfnGetClientLanguageCode
- GfnGetClientCountryCode
- GfnGetClientInfo
- GfnGetPartnerSecureData
- GfnFree
- GfnSetEditBoxRegion
- GfnClearEditBoxRegion
- GfnSetActionZone

## CloudCheckAPI
This C-based simple command-line sample demonstrates usage of all the various native and REST APIs dedicated to checking if running in the GFN cloud environment.
For the HTTP REST endpoint calls, the sample leverages the utility functions provided in the GfnSocketUtils sources under ./samples/common
It is designed to be run in both client and GeForce NOW cloud environments to provide expected results in each of the environments for each of the APIs.

### Intended Audience
The sample is intended for game and launcher developers that want their games or applications to know when they are running in the GeForce NOW cloud.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnCloudCheck, both basic and secure calling methods
- GfnGetCloudType
- HTTP REST endpoint gfnsdk.nvgeforcenow.com/CloudCheck

## CubeSample
This is a modified variant of Vulkan Cube app originally distributed with Vulkan SDK. It demonstrates the SDK's two-way communication between an application running inside GeForce NOW with an application running on the client device.
For ease of API review, the bulk of the integration is isolated to the GfnSdkInterface.* sources. See the sample [README](./CubeSample/README.md) for more details.

### Intended Audience
The sample is intended for game and launcher developers that need their games and applications communicating with each other across GeForce NOW without having to create a custom two-way communication stack.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnSendMessage
- GfnRegisterMessageCallback

## OpenClientBrowser
This C-based simple command-line sample demonstrates APIs that allow applications running in GeForce NOW to open URLs on the connecting client's default browser.

### Intended Audience
The sample is intended for developers that need their games or applications running inside the GeForce NOW cloud environment to direct the user to a specific website on the client device.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnOpenURLOnClient

## PartnerDataAPI
This C-based simple command-line sample demonstrates usage of the API dedicated to obtaining partner-supplied data provided during session initialization, as well as the correct way to free the memory allocated for the data.

### Intended Audience
The sample is intended for developers that need their games or applications running inside the GeForce NOW cloud environment to obtain custom data provided as part of the session launch by the client streaming application.

### APIs Demonstrated
- GfnGetPartnerSecureData
- GfnFree

## PreWarmSample
This C-based simple command-line sample demonstrates usage of the APIs and callbacks associated with putting an application into PreWarm state, and waiting for a user session to connect.
This allows an application to preload all common data, allowing a streaming user to bypass most of the loading time and get to the main menu much faster.

### Intended Audience
The sample is intended for developers that have games or applications that will support GeForce NOW's PreWarm feature.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnRegisterSessionInitCallback
- GfnAppReady

## SessionTerminationSample
This C-based command-line sample demonstrates how an application can request graceful termination of the current GFN streaming session and handle the exit callback invoked during teardown.

### Intended Audience
The sample is intended for developers whose games or applications need to programmatically end a GFN streaming session.

### APIs Demonstrated
- GfnIsRunningInCloud
- GfnRegisterExitCallback
- GfnTerminateSession

# Building the Samples
Please see the [SDK root README](../README.md) for details on how to build the samples.

# Running the Samples
All samples can be run on both client PCs and GeForce NOW cloud systems to provide information on how the APIs work in each execution scenario.

## Running on Client PC Devices
In client scenarios, the sample applications will show how cloud check calls respond when run outside of GeForce NOW.
All the cloud-specific APIs will not be called to show how to skip such APIs in your integrations as well as avoid showing a bunch of API failures due to the wrong call environment.

## Running in GeForce NOW
Running the samples inside GeForce NOW environment will require allocating GeForce NOW test and debug resources.
Please see https://developer.geforcenow.com for documentation on how to get started with testing applications inside GeForce Now.