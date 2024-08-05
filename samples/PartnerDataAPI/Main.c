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
    return _getch();
#else if __linux__
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
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

// Example application initialization method with a call to initialize the Geforce NOW Runtime SDK.
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
        printf("Error initializing the SDK: %d\n", result);
    }
    else
    {
        // Initalization success, which the return value hints at the environment detected
        switch (result)
        {
        case gfnInitSuccessClientOnly:
            // Detected this looks like a client system (outside GFN cloud).
            // Only client-related API methods should be called.
            printf("SDK initalization success, detected client environment.\n");
            break;
        case gfnInitSuccessCloudOnly:
            // Detected this looks like a GFN cloud system (GFN server).
            // Only GFN cloud-related API methods should be called.
            printf("SDK initalization success, detected cloud environment.\n");
            break;
        default:
            // Generic success case
            printf("SDK initalization success.\n");
            break;
        }
    }

    return result;
}

// Example application shutdown method with a call to shut down the Geforce NOW Runtime SDK
void SDKShutdown()
{
    printf("\n\nShutting down GFN SDK...\n");

    // Shut down the Geforce NOW Runtime SDK. Note that it's safe to call
    // gfnShutdownRuntimeSdk even if the SDK was not initialized.
    // If a failure is returned, its worth noting, but nothing can be done about it.
    GfnError result = GfnShutdownSdk();
    printf("Shutdown call for SDK result: %d\n", result);
}

// Example method to call the basic *insecure* Cloud Check APIs
bool BasicCloudCheck()
{
    bool bIsCloudEnvironment = false;
    GfnError result = gfnSuccess;
#ifdef _WIN32
    printf("\n\nPerforming Basic Secure Cloud Check via GfnCloudCheck without Challenge and Response data...\n");
    result = GfnCloudCheck(NULL, NULL, &bIsCloudEnvironment);
#else if __linux__
    printf("\n\nPerforming Basic Cloud Check via GfnIsRunningInCloud...\n");
    result = GfnIsRunningInCloud(&bIsCloudEnvironment);
#endif    
    if (GFNSDK_FAILED(result))
    {
        // Failure case, do not rely on bIsCloudEnvironment result
        printf("Cloud Check API call returned error: %d\n", result);
        return false;
    }
    else
    {
        printf("Application %s executing in Geforce NOW Cloud environment.\n", (bIsCloudEnvironment) ? "is" : "is not");
    }
    return bIsCloudEnvironment;
}

// Example method to call the GfnPartnerData() API method. This will return any data that was
// passed into the session start request either via the PartnerData field from the call to the 
// GfnSdkStartStream() API, or via a web client launch as part of the Deeplink URL.
void GetPartnerData()
{
    printf("\n\nObtaining Partner Data...\n");

    char const* partnerData = NULL;
    GfnError result = GfnGetPartnerData(&partnerData);
    switch (result)
    {
    case gfnThrottled:
        printf("GfnGetPartnerData: API call rate limit exceeded. Try again later.\n");
        break;
    case gfnNoData:
        printf("GfnGetPartnerData: No partner data found.\n");
        break;
    case gfnSuccess:
        printf("GfnGetPartnerData: API call returned partner data:\n\n%s\n", partnerData);
        // Memory was allocated for the partner data. To avoid leaks, call GfnFree only in this case.
        GfnFree(&partnerData);
        break;
    default:
        printf("GfnGetPartnerData: API call returned error: %d\n", result);
        break;
    }
}

// Example method to call the GfnPartnerSecureData() API method. This will return any data that
// was passed into the session start request that is either passed by the session launch client
// via the GfnSdkStartStream() API or was sent in response to Deep Link nonce validation request
// from a partner's web backend.
void GetPartnerSecureData()
{
    printf("\n\nObtaining Partner Secure Data...\n");

    char const* partnerData = NULL;
    GfnError result = GfnGetPartnerSecureData(&partnerData);
    switch (result)
    {
    case gfnThrottled:
        printf("GfnGetPartnerSecureData: API call rate limit exceeded. Try again later.\n");
        break;
    case gfnNoData:
        printf("GfnGetPartnerSecureData: No partner data found.\n");
        break;
    case gfnSuccess:
        printf("GfnGetPartnerSecureData: API call returned partner secure data:\n\n%s\n", partnerData);
        // Memory was allocated for the partner secure data. To avoid leaks, call GfnFree only in this case.
        GfnFree(&partnerData);
        break;
    default:
        printf("GfnGetPartnerSecureData: API call returned error: %d\n", result);
        break;
    }
}

// Example application main
int main(int argc, char* argv[])
{
    // First step: Initialize the Geforce NOW Runtime SDK before any other SDK method calls.
    if (GFNSDK_FAILED(SDKInitialize()))
    {
        // Initialization failure, exit now, no need to call GfnShutdownSdk()
        // Calling any SDK methods in this state will return indeterministic results that should not be trusted.
        return -1;
    }
    
    // PartnerData-related APIs can only be called in the cloud environment, and so that must be checked first.
    if(!BasicCloudCheck())
    {
        printf("Not running in Geforce NOW Cloud environment, exiting early without calling Cloud APIs.\n");
        return -1;
    }

    GetPartnerData();
    
    GetPartnerSecureData();

    // GFN SDK Shutdown. It's safe to call ShutdownSDK even if the SDK was not initialized.
    SDKShutdown();

    // Ready for application exit based on Spacebar press.
    waitForSpaceBar();

    return 0;
}
