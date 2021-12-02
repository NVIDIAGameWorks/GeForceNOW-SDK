This directory contains sample projectors that demonstrate use of the GeForce Now (GFN) Runtime API.

Sample Overview:
====================================================================================================
CGameAPISample:
    This sample demonstrates usage of the application-focused APIs to control behavior of a
    title running in the GeForce NOW environment. The sample focuses on use of callbacks to notify
    a title of environment state changes.

SDKDllDirectRefSample:
    This sample demonstrates basic SDK usage without relying on the wrapper helper functions. This
    can be useful for partners who are unable to utilize the wrapper in their environment.

SampleService:
    This GFN-ready Windows service demonstrates how to call gfnIsRunningInCloudSecure API from a 
    non-elevated process.
    
SampleLauncher:
    This sample demonstrates usage of the Launcher/Publisher-focused APIs, including getting a list
    of supported titles as well as invoking GeForce NOW to start a streaming session of a title. 
    
    In addition, this sample is CEF-based to provide a view of how a CEF-based Launcher application
    can make use of the API. Sample source is a fork of the CEF project's example source found at
    https://bitbucket.org/chromiumembedded/cef-project/src.
    
    For applications that are not CEF-based, users can focus on API calls as found in the 
    src/gfn_sdk_demo/gfn_sdk_helper.cc file.

Building:
====================================================================================================
Prerequisites:
    Visual Studio 2015 or later is required to build the samples against the GFN Runtime API static
    libraries.

    Version 3.11 or later of cmake or later is required to generate the Visual Studio solution.
    Please visit( https://cmake.org/download/ ) to install or update.

Generating Visual Studio Solution:
    There is a BATCH file at the root of the distribution package that provides sample generation of
    GfnRuntimeSdk.sln against your latest Visual Studio installation with x86 Debug target in a new
    folder called "build". For more advanced or custom usage, invoke cmake directly. Example for 
    generating a Visual Studio 2017 solution with x64 targets:
        mkdir build
        cd build
        cmake .. -G "Visual Studio 15 2017" -A x64

Compiling:
    Once the solution is built, simply run or debug the samples, as the generator will correctly 
    set links back to the GFN Runtime API include and lib folders.


SampleService
====================================================================================================
SampleService integrates GFN SDK and provides a secure IPC command to query for
cloud secure check (gfnIsRunningInCloudSecure API).  

To see this command working:
* Create and start SampleService with following steps:
    * Start a command prompt as an administrator and run
    * sc create GfnSdkSampleService binpath= <pathToSamples>\SampleService.exe displayname="GfnSdk Sample Service"
    * sc start GfnSdkSampleService
* On launch of the SampleLauncher, the application checks if the service is running and queries for 
  secure cloud check.