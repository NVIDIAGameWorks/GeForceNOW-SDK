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
#include <tchar.h>
#include <windows.h>            // For GetAsyncKeyState
#include "SampleModule.h"

bool g_MainDone = false;
int g_pause_call_counter = 0;
HMODULE gfnSdkModule = NULL;

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

// Example application initialization method with a call to initialize the Geforce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
void ApplicationInitialize()
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
}

// Example application shutdown method with a call to shut down the Geforce NOW Runtime SDK
void ApplicationShutdown()
{
    printf("\n\nApplication: Shutting down...\n");
    ShutdownRuntimeSdk();
}

// Example application main
int _tmain(int argc, _TCHAR* argv[])
{
    ApplicationInitialize();

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
                " }\n",
                info.version, info.osType, info.ipV4,
                info.country, info.locale
            );
        }
        else
        {
            printf("Failed to retrieve client info. GfnError: %d\n", (int)runtimeError);
        }
    }

    // Application main loop
    printf("\n\nApplication: In main application loop; Press space bar to exit...\n\n");
    GetAsyncKeyState(' '); // Clear space bar change bit
    while (!g_MainDone)
    {
        // Do application stuff here..
        if (GetAsyncKeyState(' ') != 0)
        {
            g_MainDone = true;
        }
    }

    // Application Shutdown
    // It's safe to call ShutdownShieldXLinkSDK even if the SDK was not initialized.
    ApplicationShutdown();

    return 0;
}

static GfnRuntimeError InitSdk()
{
    GfnRuntimeError ret = gfnSuccess;
    GfnRuntimeError clientStatus = gfnSuccess;
    // For security reasons, it is preferred to check the digital signature before loading the DLL.
    // Such code is not provided here to reduce code complexity and library size, and in favor of
    // any internal libraries built for this purpose.
#ifdef _DEBUG
    gfnSdkModule = LoadLibraryW(L".\\GfnRuntimeSdk.dll");
#else
    gfnSdkModule = gfnSecureLoadClientLibraryW(L".\\GfnRuntimeSdk.dll", 0);
#endif
    if (gfnSdkModule == NULL)
    {
        printf("Not able to load client library. LastError=0x%08X\n", GetLastError());
        ret = gfnDllNotPresent;
    }
    else
    {
        gfnInitializeRuntimeSdkSig initFn = (gfnInitializeRuntimeSdkSig)GetProcAddress(gfnSdkModule, "gfnInitializeRuntimeSdk");
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
        ((gfnShutdownRuntimeSdkSig)GetProcAddress(gfnSdkModule, "gfnShutdownRuntimeSdk"))();
    }
}

static void IsRunningInCloud(bool* bIsCloudEnvironment)
{
    *bIsCloudEnvironment = ((gfnIsRunningInCloudSig)GetProcAddress(gfnSdkModule, "gfnIsRunningInCloud"))();
}

static GfnRuntimeError RegisterClientInfoCallback(ClientInfoCallbackSig cbFn)
{
    return ((gfnRegisterClientInfoCallbackSig)GetProcAddress(gfnSdkModule, "gfnRegisterClientInfoCallback"))(cbFn, NULL);
}

static GfnRuntimeError GetClientInfo(GfnClientInfo* info)
{
    return ((gfnGetClientInfoSig)GetProcAddress(gfnSdkModule, "gfnGetClientInfo"))(info);
}
