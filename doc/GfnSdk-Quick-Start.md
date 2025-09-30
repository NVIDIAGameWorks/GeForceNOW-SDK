# GeForce NOW Software Development Kit: Quick Start Guide 

| Version | Date | Description of Change | 
| :----: | :----: | :---- |
| 1.0 | 11/09/2023 | Initial release revision |
| 2.0 | 03/04/2024 | Added CloudCheckAPI Sample information, modified secure CloudCheck example to show usage of the new GfnCloudCheck API. |
| 2.1 | 11/20/2024 | Added samples information, extended information on callback purposes and uses |
| 3.0 | 06/26/2025 | Updated to match functional improvements found in SDK 3.0 |

## Introduction
NVIDIA GeForce NOW (GFN) allows users to enjoy their library of PC games on many internet-enabled devices via the power of cloud gaming. This includes PCs, Macs, Smart TVs, and mobile devices such as smartphones and tablets. 

GFN has the ability to onboard your Windows and Linux games and applications as-is for your users to enjoy. For tighter integration of these applications with GFN, NVIDIA provides the GeForce NOW Runtime Software Development Kit (GFN SDK) to developers so that their applications can perform various tasks; from knowing when the application is running in GFN to getting information about the GFN session or the user’s client system.

This document is provided as part of the SDK to aid in getting the developer up to speed with the SDK quickly, as well as providing source code pointers for the most common of integration scenarios.

## Audience
This document is directed towards developers of applications and games that are part of the GFN ecosystem, and wish to provide their users a seamless experience between their application and GFN.

For the most complete understanding of the SDK, this document should be read to completion. If time does not permit that, then it is suggested to cover Section 3, then the pertinent integration use cases for your integration needs as found in Section 5, and review Section 8 for best practices in using SDK API functions.

## SDK Overview
The GFN Runtime SDK consists of an ever-growing collection of C-based native APIs that support both Windows and Linux applications. Along with the C source files that define the APIs, the SDK also includes various documentation to aid in development, as well as a collection of samples in source code form that provide example execution of the APIs in various scenarios.

The SDK is designed with ease of use in mind, with API design meant to allow function calls and data consumption to be straightforward. In addition, the SDK is designed to support backwards compatibility as a core requirement, which allows developers to move to new versions of the SDK without the worry of code breakage related to existing API calls due to API definition alterations or deprecation.

## Obtaining the GFN SDK
If you do not already have the GFN SDK, the distribution is made available publicly via GitHub: [https://github.com/NVIDIAGameWorks/GeForceNOW-SDK/](https://github.com/NVIDIAGameWorks/GeForceNOW-SDK/), with new releases targeting a monthly cadence. Obtaining the SDK can be accomplished either by cloning the GitHub repository, or by downloading the latest release via the “Releases” list.

## Integrating the GFN SDK
GFN SDK integration is simple; integrate C-based headers and sources into your project and call the C-based functions. There are no binaries to build or add to your project.

Integration of the SDK into a game or application supported by GeForce NOW (also known as a “GFN title”) is performed by pulling the header files and sources from ./include into the applications build source tree. No binaries need to be built or included as the APIs in these include files defer work to the binaries on the game seat. The table below provides information on these headers and sources for each supported Operating System.

| Target OS | Headers and Sources (./include) |
| :---- | :---- |
| Windows 64-bit | GfnRuntimeSdk\_Wrapper.h GfnRuntimeSdk\_Wrapper.c GfnSdk\_SecureLoadLibrary.h GfnSdk\_SecureLoadLibrary.c |
| Windows 32-bit | GfnRuntimeSdk\_Wrapper.h GfnRuntimeSdk\_Wrapper.c GfnSdk\_SecureLoadLibrary.h GfnSdk\_SecureLoadLibrary.c |
| Linux 64-bit | GfnRuntimeSdk\_Wrapper.h GfnRuntimeSdk\_Wrapper.c |

The optional “wrapper” files provide a bridge from native C/C++ to the library export functions. In addition for Windows builds, they also take care of making sure the SDK binary found in the GFN game seat is authentic by checking for the appropriate NVIDIA Digital Signatures on the game seat DLL. Use of these wrapper files is highly recommended.

## Building the Included Samples
The SDK does include the sources of several sample applications, each designed to show what specific APIs do and how to call them based on the application type. Please refer to the README.md file at the root of ./samples provides information on what each sample targets. 

These samples can be built from source for Windows x86/x64, and Linux x64 platforms. The steps and tools needed to build these samples depend on the platform they will be built for. In general, you will need cmake 3.27 or greater ([https://cmake.org](https://cmake.org)), as well as a build system manager such as ninja-builder ([https://ninja-build.org/](https://ninja-build.org/)). If building for Windows, the SDK build system is set up to take advantage of Visual Studio’s support for cmake. If not familiar with this support, please visit [https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio) for more information.

### Building from the Command Line
The use of cmake is to make use of cmake’s preset feature. If you are not familiar with this feature, please visit [https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) for more information.

In general, building the SDK Samples is a matter of calling cmake with one of the supported presets:

| Target | Command Line | 
| :---- | :---- |
| x64-windows-debug | cmake \--preset x64-windows-debug |
| x64-windows-release | cmake \--preset x64-windows-release |
| x86-windows-debug  | cmake \--preset x86-windows-debug |
| x86-windows-release | cmake \--preset x86-windows-release |
| x64-linux-debug | cmake \--preset x64-linux-debug |
| x64-linux-release | cmake \--preset x64-linux-release |

### Building from Visual Studio
Alternatively with Visual Studio’s cmake support, you can open Visual Studio, and then use the “Open a local folder” option to open the root folder of the SDK distribution. This will result in Visual Studio reading the SDK’s make files, and generating a Visual Studio solution to build and debug from the IDE.

For more detailed information and build commands for building the SDK, please refer to the README file at the root of the SDK distribution.

## Initializing the SDK
Before any SDK API function can be called, the SDK must be initialized. To do this, call the initialization function:

```c
GfnError result = GfnInitializeSdk();
if (GFNSDK_SUCCEEDED(result)) {
	// Other calls to SDK APIs expected to succeed
} else {
	// SDK is in failed state, do not call other APIs
}
```

## Checking if Running in GFN
Since games and applications do not need to be specially built to run on GeForce NOW, the most common use of the SDK is to determine if running inside the GeForce NOW cloud environment. 

The GFN SDK provides a couple of APIs to accomplish this this check, depending on the criticality that the API call result be authentic.

For non-critical checks, for example, to bypass a graphics driver check that would only be applicable to a user’s PC:
```c
bool bIsCloudEnvironment = false;
GfnError result = GfnCloudCheck(NULL, NULL, &bIsCloudEnvironment);
if (bIsCloudEnvironment && GFNSDK_SUCCEEDED(result)) {
    // Considered to be running in GFN
}
else {
    // Considered to be NOT running in GFN     
}
```

If the check is more critical to running the GFN title, for example, to disable anti-cheat mechanisms when running inside the secure GeForce NOW environment:
```c
char nonce[CLOUD_CHECK_MIN_NONCE_SIZE] = { 0 };
if (!GfnCloudCheckGenerateNonce(nonce, CLOUD_CHECK_MIN_NONCE_SIZE)) {
    // Nonce generation failed, do not proceed with the API call
}
else {
    GfnCloudCheckChallenge challenge = { nonce, CLOUD_CHECK_MIN_NONCE_SIZE };
    GfnCloudCheckResponse response = { NULL, 0 };
    bool bIsCloudEnvironment = false;
    GfnError result = GfnCloudCheck(&challenge, &response, &bIsCloudEnvironment);
	if (bIsCloudEnvironment && GFNSDK_SUCCEEDED(result)) {
	    // Looks like GFN, verify response to make sure it isn't a tampered response
		if (response.attestationData == NULL) {
            // Response not expected to be NULL, consider invalid and not running in GFN
        }
        else {
            // Response data returned. Make use of the Cloud Check Helper utilities to validate the JSON-based data.
            if (GfnCloudCheckVerifyAttestationData(response.attestationData, challenge.nonce, challenge.nonceSize)) {
                // Response data is valid, can rely on the Cloud Check result. This is GFN!
            }
            else {
                // Response data is invalid, consider  not running in GFN
            }
            // Make sure to free the response data to avoid a leak!
            GfnFree(&response.attestationData);
        }
}
```

## Shutting Down the SDK
When finished calling all SDK API functions needed, the SDK must also be shut down and unloaded. This is accomplished with a call to gfnShutdownSdk:

```c
GfnShutdownSdk();
```

That’s it, no return values to handle, no binaries to manually unload.

## Examples for Other APIs
The SDK is design to have a consistent design for invocation of the API functions, which allows the code examples above to provide an overview of what to expect on calling and handling other API calls. For specifics for each API, please refer to the SDK API documentation and the source code of the sample applications.

## SDK Usage Best Practices

This section includes helpful development tips for using the SDK in various cases:

### Memory Management {#memory-management}

As mentioned with example code scenarios in Section 5, certain API functions will allocate memory for variable-length data returned by the function. This approach allows more dynamic data use without setting static limits of data, however, this also means the memory must be managed. When the documentation mentions memory is allocated for a specific function, keep in mind for proper memory management.

* **DO** check the return value of the API function for succes or failure. The API function will only allocate memory in a success case.  
* **DO** call GfnFree when done with the data in the memory. This includes callback cases.  
* **DO NOT** call the same API function again with the same variable before freeing the memory from the previous call. This will leak memory.  
* **DO NOT** blindly call GfnFree after an API function call. Per the first bullet, if the API function returned any error, including gfnNoData, no memory is allocated, and the pointer is uninitalized. Attempting to free uninitialized memory can result in Access Violation or other unhandled exceptions.

### Multiple Concurrent Instance Handling

The SDK does support multiple instances of the SDK across multiple processes, for example, a game launcher and a game process both initializing the SDK and calling API functions. However, in most cases, concurrency is not needed; a single process could initialize and call API functions and pass the data between processes to make sure the process’s stay in sync in regards to SDK data.

### Multiple Calls to APIs That Return Static Data

The SDK supports calling an API repeatedly, however, most APIs return static data, which is data that is not going to change during process lifetime. For example, if GfnCloudCheck returns true during a session, that value is not going to change during the calling process’s lifetime. In such cases, it is more efficient to call the API once and cache the value for future use than to call the API again.

### Thread Safety

Several SDK API functions either work asynchronously, or have an asynchronous variant. When calling such functions or registering for callbacks in multi-threaded processes, it is important to keep the calls and returned data straight. For example, care must be taken when one thread calls GfnGetPartnerData that allocates memory, and another thread is tasked with the call to GfnFree. In such cases, it is more robust to call GfnFree from the thread that called GfnGetPartnerData. Similar care must be taken when calling synchronous functions that block a thread while other threads continue to execute.

### Complex Data into Generic Data Types

The SDK APIs are C-based for the widest range of compatibility to common game engines and other frameworks. This means that the APIs define common C-based generic data types for many parameters, for example, char\*. This requires certain application-defined complex data objects, such as a JSON object, be stringified before passing into certain functions, or de-stringified when read from functions that return such data. Performing a type cast on these parameters versus turning the data into string form can have negative results on the data, or even cause an application crash.

### Callback Handling

The SDK’s callbacks provide powerful methods to respond to changing states and data of the streaming session. It is encouraged to use them, however, keep in mind the execution of the callback function handlers should not be long-running or blocking functions. Many of the callbacks have timeouts. If the work required to process a callback is expected to take significant time, it is recommended to spin that work off as a separate thread and return from the callback function as quickly as possible.

## Conclusion and Next Steps

This document is meant to provide an overview of the SDK and the most-used API methods. The code samples provided here are meant to provide enough code to get started, and is not complete drop-in code. For the sake of brevity, not all error cases are handled by the sample code provided, whereas production-level code must do so.

For complete information about the APIs, refer to the API documentation in the ./doc folder, as well as the working code supplied with the samples in ./samples. If there are still questions about the SDK, then please email the SDK development team at [geforcenow-sdk-support@nvidia.com](mailto:geforcenow-sdk-support@nvidia.com).
