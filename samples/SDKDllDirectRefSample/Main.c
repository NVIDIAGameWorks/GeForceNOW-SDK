// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2019-2021 NVIDIA Corporation. All rights reserved.

#include <stdio.h>
#include <string.h>
#include "SampleModule.h"

#ifdef _WIN32
#include <tchar.h>
#include "GfnSdk_SecureLoadLibrary.h"
#define PLATFORM_MAX_PATH MAX_PATH
typedef wchar_t CHAR_TYPE;

#elif defined(__linux__)
#include <unistd.h>
#include <termios.h>
#include <dlfcn.h>      // dlopen
#include <libgen.h>     // dirname
#include <limits.h>     // PATH_MAX
#define PLATFORM_MAX_PATH PATH_MAX
typedef char CHAR_TYPE;
typedef void* HANDLE_TYPE;
#else
#error "Unsupported platform"
#endif

HANDLE_TYPE gfnSdkModule = NULL;
bool g_MainDone = false;
int g_pause_call_counter = 0;


typedef GfnRuntimeError(*gfnInitializeRuntimeSdkSig)(GfnDisplayLanguage displayLanguage);
typedef void (*gfnShutdownRuntimeSdkSig)(void);
typedef bool (*gfnIsRunningInCloudSig)();
typedef GfnRuntimeError(*gfnGetClientInfoSig)(GfnClientInfo*);
typedef GfnRuntimeError(*gfnRegisterClientInfoCallbackSig)(ClientInfoCallbackSig, void*);

static GfnRuntimeError InitSdk();
static void ShutdownRuntimeSdk();
static void IsRunningInCloud(bool* bIsCloudEnvironment);
static GfnRuntimeError GetClientInfo(GfnClientInfo* info);
static GfnRuntimeError RegisterClientInfoCallback(ClientInfoCallbackSig cbFn);

static void GetSharedLibraryPath(CHAR_TYPE* path, size_t bufferLen);
static HANDLE_TYPE LoadSdkLibrary();
static void* GetSymbol(HANDLE_TYPE library, char* name);
static void ApplicationMainLoop();

// Example application initialization method with a call to initialize the Geforce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
bool ApplicationInitialize()
{
    printf("\n\nApplication: Initializing...\n");

    // Initialize the Geforce NOW Runtime SDK using the C calling convention.
    GfnRuntimeError err;

    err = InitSdk();
    if (GFNSDK_FAILED(err))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        printf("Error initializing the sdk: %d\n", err);
        return false;
    }
    else
    {
        // If we're running in the cloud environment, initialize cloud callbacks so we can 
        // receive events from the server. These are not used in client only mode.
        bool bIsCloudEnvironment = false;
        IsRunningInCloud(&bIsCloudEnvironment);
        if (bIsCloudEnvironment)
        {
            err = RegisterClientInfoCallback(HandleClientDataChanges);
            if (err != gfnSuccess)
            {
                printf("Error registering clientInfo callback: %d\n", err);
            }
        }
    }

    // Application Initialization here
    return true;
}

// Example application shutdown method with a call to shut down the Geforce NOW Runtime SDK
void ApplicationShutdown()
{
    printf("\n\nApplication: Shutting down...\n");
    ShutdownRuntimeSdk();
}

// Example application main
#ifdef _WIN32
int wmain(int argc, CHAR_TYPE* argv[])
#elif defined(__linux__)
int main(int argc, CHAR_TYPE* argv[])
#endif
{
    if (!ApplicationInitialize()) {
        printf("Failed to initialize! Exiting...");
        ApplicationShutdown();
        return -1;
    }

    // Sample C API call
    bool bIsCloudEnvironment = false;
    IsRunningInCloud(&bIsCloudEnvironment);
    printf("\nApplication executing in Geforce NOW environment: %s\n", (bIsCloudEnvironment == true) ? "true" : "false");

    if (bIsCloudEnvironment) // More sample C API calls.
    {
        GfnError runtimeError = gfnSuccess;
        GfnClientInfo info;
        runtimeError = GetClientInfo(&info);
        if (runtimeError == gfnSuccess)
        {
            printf("GetClientInfo returned: { version: %d, osType: %d, ipV4: %s, "
                "country: %s, locale:%s"
                ", RTDAverageLatencyMs: %d"
                " }\n",
                info.version, info.osType, info.ipV4,
                info.country, info.locale
                , info.RTDAverageLatencyMs
            );
        }
        else
        {
            printf("Failed to retrieve client info. GfnError: %d\n", (int)runtimeError);
        }
    }

    // Application main loop
    ApplicationMainLoop();

    // Application Shutdown
    // It's safe to call ShutdownShieldXLinkSDK even if the SDK was not initialized.
    ApplicationShutdown();

    return 0;
}

static GfnRuntimeError InitSdk()
{
    GfnRuntimeError ret = gfnSuccess;
    GfnRuntimeError clientStatus = gfnSuccess;
    
    gfnSdkModule = LoadSdkLibrary();
    if (gfnSdkModule == NULL)
    {
        ret = gfnDllNotPresent;
    }
    else
    {
        gfnInitializeRuntimeSdkSig initFn = (gfnInitializeRuntimeSdkSig)GetSymbol(gfnSdkModule, "gfnInitializeRuntimeSdk");
        if (initFn == NULL)
        {
            ret = gfnAPINotFound;
        }
        else
        {
            ret = (initFn)(gfnDefaultLanguage);
        }
    }
    // gfnDllNotPresent means client dll was not present, this is allowed in the cloud environment but not suggested.
    // Any other error is fatal.
    if (GFNSDK_FAILED(ret) && ret != gfnDllNotPresent)
    {
        printf("Client library init failed: %d\n", clientStatus);
        if (!!gfnSdkModule)
        {
            ShutdownRuntimeSdk();
        }
    }
    return ret;
}

static void ShutdownRuntimeSdk()
{
    if (!!gfnSdkModule)
    {
        ((gfnShutdownRuntimeSdkSig)GetSymbol(gfnSdkModule, "gfnShutdownRuntimeSdk"))();
    }
}

static void IsRunningInCloud(bool* bIsCloudEnvironment)
{
    *bIsCloudEnvironment = ((gfnIsRunningInCloudSig)GetSymbol(gfnSdkModule, "gfnIsRunningInCloud"))();
}

static GfnRuntimeError RegisterClientInfoCallback(ClientInfoCallbackSig cbFn)
{
    return ((gfnRegisterClientInfoCallbackSig)GetSymbol(gfnSdkModule, "gfnRegisterClientInfoCallback"))(cbFn, NULL);
}

static GfnRuntimeError GetClientInfo(GfnClientInfo* info)
{
    return ((gfnGetClientInfoSig)GetSymbol(gfnSdkModule, "gfnGetClientInfo"))(info);
}

static void GetSharedLibraryPath(CHAR_TYPE* path, size_t bufferLen)
{
#ifdef _WIN32
    DWORD len = 0;
    len = GetFullPathNameW(L".\\GfnRuntimeSdk.dll", bufferLen, path, NULL);
#elif defined(__linux__)
    char buffer[PLATFORM_MAX_PATH];
    ssize_t len;

    // Get the absolute path of the executable
    len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

    // Check for errors
    if (len == -1) {
        return;
    }

    // Null-terminate the string
    buffer[len] = '\0';

    // Get the directory path, excluding the executable name
    char* directory = dirname(buffer);
    strncpy(path, directory, PLATFORM_MAX_PATH);

    // Append "GfnRuntimeSdk.so" to the directory path
    if (strlen(path) + strlen("/GfnRuntimeSdk.so") + 1 > PLATFORM_MAX_PATH) {
        printf("ERROR: Could not get default client library path name: Path too long");
        return;
    }

    strcat(path, "/GfnRuntimeSdk.so");
#endif
} 

static HANDLE_TYPE LoadSdkLibrary()
{
    HANDLE_TYPE handle = NULL;
    CHAR_TYPE libpath[PLATFORM_MAX_PATH];
    memset(libpath, 0, sizeof(libpath) / sizeof(CHAR_TYPE));
    GetSharedLibraryPath(libpath, PLATFORM_MAX_PATH);

    // For security reasons, it is preferred to check the digital signature before loading the DLL.
    // Such code is not provided here to reduce code complexity and library size, and in favor of
    // any internal libraries built for this purpose.
#ifdef _WIN32
#   ifdef _DEBUG
        handle = LoadLibraryW(libpath);
#   else
        handle = gfnSecureLoadClientLibraryW(libpath, 0);
#   endif
#elif __linux__
    handle = dlopen(libpath, RTLD_LAZY);
#endif

    if (!handle)
    {
#ifdef _WIN32
        printf("Not able to load client library. LastError=0x%08X\n", GetLastError());
#elif __linux__
        printf("Not able to load client library. dlerror=%s\n", dlerror());
#endif
    }

    return handle;
}

static void ApplicationMainLoop()
{
    printf("\n\nApplication: In main application loop; Press space bar to exit...\n\n");
#ifdef _WIN32
    GetAsyncKeyState(' '); // Clear space bar change bit
    while (!g_MainDone)
    {
        // Do application stuff here..
        if (GetAsyncKeyState(' ') != 0)
        {
            g_MainDone = true;
        }
    }
#elif defined(__linux__)
    char ch;

    struct termios orig_termios;
    struct termios new_termios;

    // make sure to flush the input to prevent any old input from being read
    tcflush(0, TCIFLUSH);

    // Get the current settings (which we will restore)
    tcgetattr(0, &orig_termios);
    
    // Copy the settings to new_termios and modify them
    new_termios = orig_termios;
    new_termios.c_lflag &= ~ICANON; // Disable canonical mode
    new_termios.c_lflag &= ~ECHO; // Disable echo
    new_termios.c_cc[VMIN] = 1; // Set minimum read characters to 1
    new_termios.c_cc[VTIME] = 0; // Set timeout to 0 deciseconds

    // Set the new attributes
    tcsetattr(0, TCSANOW, &new_termios);

    while (!g_MainDone)
    {
        // Do application stuff here..
        if (read(0, &ch, 1) < 0)
        {
            // No key read
            usleep(100000); // Sleep for 100ms
            continue;
        }

        if (ch == ' ')
        {
            g_MainDone = true;
        }
    }

    tcsetattr(0, TCSANOW, &orig_termios);
#endif
}
