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

// Leverage the Sample Helpers for GfnCloudCheck() challenge and response data
#include "GfnCloudCheckUtils.h"

#ifdef _WIN32
#   include <conio.h>
#elif __linux__
#   include <termios.h>
#   include <unistd.h>
#endif

// Defined the minimum allowable size for the nonce used for GfnCloudCheck()
#define CLOUD_CHECK_MIN_NONCE_SIZE 16

// Keyboard input helper function
static char getKeyPress() {
#ifdef _WIN32
    return _getch();
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
void BasicCloudCheck()
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
}

// Example method to call the GfnCloudCheck() API method without optional challenge and response data.
// This gives a result with a basic level of security about the validity of the response
void BasicSecureCloudCheck()
{
    printf("\n\nPerforming Basic Secure Cloud Check via GfnCloudCheck without Challenge and Response data...\n");

    bool bIsCloudEnvironment = false;
    GfnError result = GfnCloudCheck(NULL, NULL, &bIsCloudEnvironment);
    if (GFNSDK_FAILED(result))
    {
        // Failure case, report various important cases
        if (result == gfnThrottled)
        {
            printf("GfnCloudCheck: API call rate limit exceeded. Try again later.\n");
        }
        else
        {
            printf("GfnCloudCheck: API call returned error: %d, %s\n", result, GfnErrorToString(result));
        }
    }
    else
    {
        printf("GfnCloudCheck: Application %s executing in GeForce NOW Cloud environment.\n", (bIsCloudEnvironment) ? "is" : "is not");
    }
}

// Example method to call the GfnCloudCheck() API method without optional challenge and response data.
// This gives a result with a base level of security about the validity of the response
void ExtendedSecureCloudCheck()
{
    printf("\n\nPerforming Basic Secure Cloud Check via GfnCloudCheck with Challenge and Response data...\n");

    // Generate a nonce, the minimum nonce size requirement is 16 bytes.
    char nonce[CLOUD_CHECK_MIN_NONCE_SIZE] = { 0 };
    if (!GfnCloudCheckGenerateNonce(nonce, CLOUD_CHECK_MIN_NONCE_SIZE))
    {
        // Nonce generation failed, do not proceed with the API call
        printf("GfnCloudCheckGenerateNonce failed, GfnCloudCheck() call skipped.\n");
        return;
    }

    GfnCloudCheckChallenge challenge = { nonce, CLOUD_CHECK_MIN_NONCE_SIZE };
    GfnCloudCheckResponse response = { NULL, 0 };
    bool bIsCloudEnvironment = false;
    GfnError result = GfnCloudCheck(&challenge, &response, &bIsCloudEnvironment);
    if (GFNSDK_FAILED(result))
    {
        // Failure case, report various important cases
        if (result == gfnThrottled)
        {
            printf("GfnCloudCheck: API call rate limit exceeded. Try again later.\n");
        }
        else
        {
            printf("GfnCloudCheck: API call returned error: %d, %s\n", result, GfnErrorToString(result));
        }
        // Function failure means that it cannot be securely considered GFN
        printf("GfnCloudCheck: Considered not running in GFN.\n");
        return;
    }

    if (!bIsCloudEnvironment)
    {
        // Not cloud, no need to validate response data, as it will be null
        printf("GfnCloudCheck: Application is not executing in GeForce NOW Cloud environment, skipped response data validation.\n");
    }
    else
    {
        // GfnCloudCheck returned the environment is GFN Cloud, but need to use response data to verify.
        if (response.attestationData == NULL)
        {
            // Response not expected to be NULL, consider invalid
            printf("GfnCloudCheck: Response data is null, not valid. Considered not running in GFN.\n");
        }
        else
        {
            // Response data returned. Make use of the Cloud Check Helper utilities to validate the JSON-based data.
            if (GfnCloudCheckVerifyAttestationData(response.attestationData, challenge.nonce, challenge.nonceSize))
            {
                printf("GfnCloudCheck: Response data is valid.\n");
                // Since the response data is valid, can rely on the Cloud Check result.
                printf("GfnCloudCheck: Application is running in GeForce NOW Cloud environment with high level of confidence.\n");
            }
            else
            {
                printf("GfnCloudCheck: Response data is not valid. Considered not running in GFN.\n");
            }
            // Make sure to free the response data to avoid a leak!
            GfnFree(&response.attestationData);
        }
    }
}


// Example application main
int main(int argc, char* argv[])
{
    // First step: Initialize the GeForce NOW Runtime SDK before any other SDK method calls.
    if (GFNSDK_FAILED(SDKInitialize()))
    {
        // Initialization failure, exit now, no need to call GfnShutdownSdk()
        // Calling any SDK methods in this state will return indeterministic results that should not be trusted.
        return -1;
    }

    BasicCloudCheck();
    BasicSecureCloudCheck();
    ExtendedSecureCloudCheck();

    // GFN SDK Shutdown. It's safe to call ShutdownSDK even if the SDK was not initialized.
    SDKShutdown();

    // Ready for application exit based on Spacebar press.
    waitForSpaceBar();

    return 0;
}
