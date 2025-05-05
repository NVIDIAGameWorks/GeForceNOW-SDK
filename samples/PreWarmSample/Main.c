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
// Copyright (c) 2024 NVIDIA Corporation. All rights reserved.

#include <stdio.h>

// Sample will use the Helper Wrapper sources to auto-manage SDK library handling
#include "GfnRuntimeSdk_Wrapper.h"

#ifdef _WIN32
#   include <conio.h>
#elif __linux__
#   include <termios.h>
#   include <unistd.h>
#endif

// Keyboard input helper function
static char getKeyPress() {
#ifdef _WIN32
    return (char)_getch();
#elif __linux__
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#else
#error "Unsupported platform"
#endif
}

// Loop exit helper function that waits for spacebar press
static void waitForSpaceBar() {
    printf("\n\nPress space bar to exit...\n\n");
    char c;
    do
    {
        c = getKeyPress();
    } while (c != ' ');
}

// Example application initialization method with a call to initialize the GeForce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
GfnError SDKInitialize()
{
    // Using gfnDefaultLanguage to tell the SDK to use the default system language for any
    // UI it might show, which won't be the case in this sample.
    printf("\n\nInitializing GFN SDK...\n");
    GfnError result = GfnInitializeSdk(gfnDefaultLanguage);
    if (GFNSDK_FAILED(result))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        printf("Error initializing the SDK: %d, %s\n", result, GfnErrorToString(result));
    }
    else
    {
        // Initialization success, which the return value hints at the environment detected
        switch (result)
        {
        case gfnInitSuccessClientOnly:
            // Detected this looks like a client system (outside GFN cloud).
            // Only client-related API methods should be called.
            printf("SDK initialization success, detected client environment.\n");
            break;
        case gfnInitSuccessCloudOnly:
            // Detected this looks like a GFN cloud system (GFN server).
            // Only GFN cloud-related API methods should be called.
            printf("SDK initialization success, detected cloud environment.\n");
            break;
        default:
            // Generic success case
            printf("SDK initialization success.\n");
            break;
        }
    }

    return result;
}

// Example application shutdown method with a call to shut down the GeForce NOW Runtime SDK
void SDKShutdown()
{
    printf("\n\nShutting down GFN SDK...\n");

    // Shut down the GeForce NOW Runtime SDK. Note that it's safe to call
    // gfnShutdownRuntimeSdk even if the SDK was not initialized.
    // If a failure is returned, its worth noting, but nothing can be done about it.
    GfnError result = GfnShutdownSdk();
    printf("Shutdown call for SDK result: %d, %s\n", result, GfnErrorToString(result));
}

// Example method to call the basic *insecure* GfnIsRunningInCloud() API method
bool BasicCloudCheck()
{
    printf("\n\nPerforming Basic Cloud Check via GfnIsRunningInCloud...\n");

    bool bIsCloudEnvironment = false;
    GfnError result = GfnIsRunningInCloud(&bIsCloudEnvironment);
    if (GFNSDK_FAILED(result))
    {
        // Failure case, do not rely on bIsCloudEnvironment result
        printf("GfnIsRunningInCloud: API call returned error: %d, %s\n", result, GfnErrorToString(result));
    }
    else
    {
        printf("GfnIsRunningInCloud: Application %s executing in GeForce NOW Cloud environment.\n", (bIsCloudEnvironment) ? "is" : "is not");
    }

    return bIsCloudEnvironment;
}

GfnApplicationCallbackResult GFN_CALLBACK SessionInit(const char* params, void* pContext)
{
    (void)pContext;
    // Callback for when GeForce NOW a user connects to the game seat to start a streaming session.
    // Since a user is connected, now user data can be loaded
    // Respond within 30 seconds with a call to gfnAppReady API
    printf("SessionInit: %s\n", params);
    printf("Loading user data now...\n");
    // Report that the loading was successful and application is ready for streaming to begin
    GfnError result = GfnAppReady(true, "All Good!");
    if (result == gfnSuccess)
    {
        printf("Reported 'AppReady' with success to the SDK.\n");
    }
    else
    {
        printf("Failed to report 'AppReady' to the SDK: %d, %s\n", result, GfnErrorToString(result));
    }

    return crCallbackSuccess;
}

// Example application main
int main()
{
    // If the application will run in standard mode or pre-warm mode, a command line parameter can be used
    // to define the mode. If a parameter enables pre-warm, make sure to define it as part of GFN onboarding.

    // For code simplicity, the sample assumes it should always launch in pre-warm mode.

    // Initialize the GeForce NOW Runtime SDK before any other SDK method calls.
    if (GFNSDK_FAILED(SDKInitialize()))
    {
        // Initialization failure, exit now, no need to call GfnShutdownSdk()
        // Calling any SDK methods in this state will return indeterministic results that should not be trusted.
        return -1;
    }

    // Pre-warm requires to be running in GFN.
    if (!BasicCloudCheck())
    {
        printf("Not running in GFN, will not go through pre-warm work.\n");
    }
    else
    {
        // At this point, application should load all common app data.
        // The goal is to load everything possible now to shorten loading time
        // seen by the user when they connect to the system via streaming session.
        // No user data should be loaded at this time as no user is connected to the system.
        printf("Loading common (non-user) data now...\n");
        
        // Once loading is done, signal to GFN that the application is ready for a user session
        GfnError result = GfnRegisterSessionInitCallback(SessionInit, NULL);
        if (result != gfnSuccess)
        {
            printf("Error registering SessionInit callback: %d, %s\n", result, GfnErrorToString(result));
        }
    }

    // Application shutdown requires calling GFN SDK Shutdown first.
    // It's safe to call ShutdownSDK even if the SDK was not initialized.
    SDKShutdown();

    // Ready for application exit based on Spacebar press.
    waitForSpaceBar();

    return 0;
}
