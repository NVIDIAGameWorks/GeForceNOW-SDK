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

// Example application initialization method with a call to initialize the Geforce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
void ApplicationInitialize()
{
    printf("\n\nApplication: Initializing...\n");

    // Initialize the Geforce NOW Runtime SDK using the C calling convention.
    GfnRuntimeError err = GfnInitializeSdk(gfnDefaultLanguage);
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
        GfnIsRunningInCloud(&bIsCloudEnvironment);
        if (bIsCloudEnvironment)
        {
            // Register any implemented callbacks capable of serving requests from the SDK.
            err = GfnRegisterExitCallback(ExitApp, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering ExitApp callback: %d\n", err);
            }
            err = GfnRegisterPauseCallback(PauseApp, &g_pause_call_counter);
            if (err != gfnSuccess)
            {
                printf("Error registering PauseApp callback: %d\n", err);
            }
            err = GfnRegisterInstallCallback(InstallApp, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering InstallApp callback: %d\n", err);
            }
            err = GfnRegisterSaveCallback(AutoSave, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering AutoSave callback: %d\n", err);
            }
            err = GfnRegisterSessionInitCallback(SessionInit, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering SessionInit callback: %d\n", err);
            }
            err = GfnRegisterClientInfoCallback(HandleClientDataChanges, NULL);
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

    // Shut down the Geforce NOW Runtime SDK. Note that it's safe to call
    // gfnShutdownRuntimeSdk even if the SDK was not initialized.
    GfnShutdownSdk();
}

// Example application main
int _tmain(int argc, _TCHAR* argv[])
{
    ApplicationInitialize();

    // Sample C API call
    bool bIsCloudEnvironment = false;
    GfnIsRunningInCloud(&bIsCloudEnvironment);
    printf("\nApplication executing in Geforce NOW environment: %s\n", (bIsCloudEnvironment == true) ? "true" : "false");

    if (bIsCloudEnvironment) // More sample C API calls.
    {
        GfnError runtimeError = gfnSuccess;

        char* clientIp;
        runtimeError = GfnGetClientIpV4(&clientIp);
        if (runtimeError == gfnSuccess)
        {
            printf("Retrieved Geforce NOW Client I.P.: %s\n", clientIp);
        }
        else
        {
            printf("Failed to retrieve Geforce NOW Client I.P. GfnError: %d\n", (int) runtimeError);
        }

        char* clientLanguageCode;
        runtimeError = GfnGetClientLanguageCode(&clientLanguageCode);
        if (runtimeError == gfnSuccess)
        {
            printf("Retrieved Geforce NOW client language code: %s\n", clientLanguageCode);
        }
        else
        {
            printf("Failed to retrieve Geforce NOW client language code. GfnError: %d\n", (int) runtimeError);
        }

        char clientCountryCode[3];
        runtimeError = GfnGetClientCountryCode(clientCountryCode, 3);
        if (runtimeError == gfnSuccess)
        {
            printf("Retrieved Geforce NOW client Country code: %s\n", clientCountryCode);
        }
        else
        {
            printf("Failed to retrieve Geforce NOW client Country code. GfnError: %d\n", (int)runtimeError);
        }

        GfnClientInfo info;
        runtimeError = GfnGetClientInfo(&info);
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


        // Try "setting up" a title!
        runtimeError = GfnSetupTitle("Sample C App");
        if (runtimeError == gfnSuccess)
        {
            printf("Setup GFN Title\n");
        }
        else
        {
            printf("Failed to setup GFN Title: %d\n", (int)runtimeError);
        }
        runtimeError = GfnAppReady(false, "Abcd");
        if (runtimeError == gfnSuccess)
        {
            printf("Reported 'AppReady' with failure to the SDK\n");
        }
        else
        {
            printf("Failed to report 'AppReady' to the SDK: %d\n", (int)runtimeError);
        }

        runtimeError = GfnAppReady(true, NULL);
        if (runtimeError == gfnSuccess)
        {
            printf("Reported 'AppReady' with success to the SDK\n");
        }
        else
        {
            printf("Failed to report 'AppReady' to the SDK: %d\n", (int)runtimeError);
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
