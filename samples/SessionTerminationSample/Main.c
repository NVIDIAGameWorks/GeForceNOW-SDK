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
// Copyright (c) 2026 NVIDIA Corporation. All rights reserved.

// This sample demonstrates how a partner application can programmatically request
// termination of the current GeForce NOW streaming session using gfnTerminateSession.
//
// Use case: the application has reached a natural stopping point (e.g., the user
// completed a game segment or triggered a logout) and wants to end the session
// rather than waiting for the user to close the stream manually.
//
// The session is torn down gracefully: GFN will invoke the registered ExitCallback
// before the session ends, giving the application an opportunity to save state.

#include <stdio.h>
#include <string.h>

// Sample will use the Helper Wrapper sources to auto-manage SDK library handling
#include "GfnRuntimeSdk_Wrapper.h"

#ifdef _WIN32
#   include <conio.h>
#   include <windows.h>
#elif __linux__
#   include <termios.h>
#   include <unistd.h>
#endif

// Upper bound on how long to wait for the ExitCallback after a termination
// request. GFN tears the session down on its own schedule, so the application
// must not block forever if the callback never arrives.
#define EXIT_CALLBACK_TIMEOUT_SECONDS 30
#define EXIT_CALLBACK_POLL_MS 100

// ---------------------------------------------------------------------------
// Keyboard / console helpers
// ---------------------------------------------------------------------------

static char GetKeyPress()
{
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

static void WaitForKeyPress(char key)
{
    char c;
    do
    {
        c = GetKeyPress();
    } while (c != key);
}

static void DoSleepMs(int milliseconds)
{
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#elif __linux__
    usleep((useconds_t)milliseconds * 1000);
#endif
}

// ---------------------------------------------------------------------------
// SDK lifecycle helpers
// ---------------------------------------------------------------------------

static GfnError SDKInitialize()
{
    printf("\nInitializing GFN SDK...\n");
    GfnError result = GfnInitializeSdk();
    if (GFNSDK_FAILED(result))
    {
        printf("Error initializing the SDK: %d, %s\n", result, GfnErrorToString(result));
    }
    else
    {
        switch (result)
        {
        case gfnInitSuccessClientOnly:
            printf("SDK initialization success, detected client environment.\n");
            break;
        case gfnInitSuccessCloudOnly:
            printf("SDK initialization success, detected cloud environment.\n");
            break;
        default:
            printf("SDK initialization success.\n");
            break;
        }
    }
    return result;
}

static void SDKShutdown()
{
    printf("\nShutting down GFN SDK...\n");
    GfnError result = GfnShutdownSdk();
    printf("Shutdown result: %d, %s\n", result, GfnErrorToString(result));
}

static bool BasicCloudCheck()
{
    printf("\nChecking cloud environment via GfnIsRunningInCloud...\n");
    bool bIsCloud = false;
    GfnError result = GfnIsRunningInCloud(&bIsCloud);
    if (GFNSDK_FAILED(result))
    {
        printf("GfnIsRunningInCloud error: %d, %s\n", result, GfnErrorToString(result));
    }
    else
    {
        printf("Application %s running in GeForce NOW Cloud environment.\n",
               bIsCloud ? "IS" : "is NOT");
    }
    return bIsCloud;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

// Set from the SDK's callback thread, polled by the main thread.
static volatile bool s_bExitCallbackInvoked = false;

// ExitCallback is invoked by the GFN SDK as part of graceful session teardown,
// including teardown triggered by GfnTerminateSession. The application should
// save any user state here before returning.
static GfnApplicationCallbackResult GFN_CALLBACK OnExit(void* pContext)
{
    (void)pContext;
    printf("\n[ExitCallback] GFN is ending the streaming session.\n");
    printf("[ExitCallback] Saving user state...\n");
    // TODO: save any unsaved user progress / state here
    printf("[ExitCallback] User state saved. Returning from exit callback.\n");
    fflush(stdout);
    s_bExitCallbackInvoked = true;
    return crCallbackSuccess;
}

// Blocks until the ExitCallback fires or the timeout expires. The SDK delivers
// the callback on an internal thread, so the SDK must stay initialized until it
// arrives — shutting down earlier drops the application off the message bus and
// the callback is never delivered.
static void WaitForExitCallback()
{
    printf("\nWaiting up to %d seconds for the ExitCallback...\n", EXIT_CALLBACK_TIMEOUT_SECONDS);

    const int maxPolls = (EXIT_CALLBACK_TIMEOUT_SECONDS * 1000) / EXIT_CALLBACK_POLL_MS;
    for (int i = 0; i < maxPolls && !s_bExitCallbackInvoked; ++i)
    {
        DoSleepMs(EXIT_CALLBACK_POLL_MS);
    }

    if (s_bExitCallbackInvoked)
    {
        printf("ExitCallback received. Safe to shut down the SDK.\n");
    }
    else
    {
        printf("Timed out waiting for the ExitCallback. Shutting down anyway.\n");
    }
}

// ---------------------------------------------------------------------------
// Session termination demo
// ---------------------------------------------------------------------------

// Requests GFN to terminate the current streaming session.
// pchReason is optional — pass NULL or empty string if no reason applies.
// Returns true if GFN accepted the request and teardown is now in progress.
static bool DemoTerminateSession(const char* pchReason)
{
    printf("\nRequesting session termination");
    if (pchReason && pchReason[0] != '\0')
    {
        printf(" with reason: \"%s\"", pchReason);
    }
    printf("...\n");

    GfnError result = GfnTerminateSession(pchReason);
    if (result == gfnSuccess)
    {
        printf("Session termination request accepted by GFN SDK.\n");
        printf("GFN will invoke the ExitCallback and then tear down the session.\n");
        return true;
    }

    printf("GfnTerminateSession failed: %d, %s\n", result, GfnErrorToString(result));
    return false;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main()
{
    // Initialize the GeForce NOW Runtime SDK before any other SDK method calls.
    if (GFNSDK_FAILED(SDKInitialize()))
    {
        // Initialization failure — no need to call GfnShutdownSdk().
        return -1;
    }

    bool bIsCloud = BasicCloudCheck();

    if (!bIsCloud)
    {
        printf("\nNot running in GFN cloud. GfnTerminateSession is a cloud-only API.\n");
        printf("Please see https://developer.geforcenow.com for more information.\n");
    }
    else
    {
        // Register an exit callback so the application is notified before the
        // session ends. This callback will be invoked as part of the graceful
        // teardown initiated by GfnTerminateSession.
        GfnError result = GfnRegisterExitCallback(OnExit, NULL);
        if (result != gfnSuccess)
        {
            printf("Warning: failed to register ExitCallback: %d, %s\n",
                   result, GfnErrorToString(result));
        }
        else
        {
            printf("ExitCallback registered successfully.\n");
        }

        // Simulate the application reaching a point where it wants to end the session.
        // In a real application this would be triggered by game logic — for example:
        //   - User completes a game segment and the partner decides to end the session
        //   - User logs out from inside the application
        //   - Application-level idle timeout expires
        printf("\nPress 'T' to request session termination with a reason string.\n");
        printf("Press 'N' to request session termination with no reason (NULL).\n");
        printf("Press any other key to skip termination and exit normally.\n");

        char key = GetKeyPress();
        bool bTerminationRequested = false;
        if (key == 't' || key == 'T')
        {
            bTerminationRequested = DemoTerminateSession("User completed game segment");
        }
        else if (key == 'n' || key == 'N')
        {
            bTerminationRequested = DemoTerminateSession(NULL);
        }
        else
        {
            printf("\nSkipping session termination. Exiting normally.\n");
        }

        if (bTerminationRequested)
        {
            WaitForExitCallback();
        }
    }

    SDKShutdown();
    printf("\nPress any key to exit...\n");
    GetKeyPress();

    return 0;
}
