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
#include "SampleModule.h"

#include "GfnCloudCheckUtils.h"

#ifdef _WIN32
#   include <conio.h>
#elif __linux__
#   include <termios.h>
#   include <unistd.h>
#endif

bool g_MainDone = false;
int g_pause_call_counter = 0;
#define CLOUD_CHECK_MIN_NONCE_SIZE 16

static char GetKeyPress() {
#ifdef _WIN32
    return (char)_getch();
#else
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

static void WaitForSpaceBar() {
    printf("\n\nApplication: In main application loop; Press space bar to exit...\n\n");
    char c;
    do
    {
        c = GetKeyPress();
    } while (c != ' ');
}

// Example application initialization method with a call to initialize the GeForce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
GfnError ApplicationInitialize()
{
    printf("\n\nApplication: Initializing...\n");

    // Initialize the GeForce NOW Runtime SDK using the C calling convention.
    GfnError result = GfnInitializeSdk();
    if (GFNSDK_FAILED(result))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        printf("Error initializing the SDK: %d, %s\n", result, GfnErrorToString(result));
    }

    return result;
}

GfnError RegisterCallbacks()
{
    GfnError overallResult = gfnSuccess;
    // Register any implemented callbacks capable of serving requests from the SDK.
    GfnError result = GfnRegisterExitCallback(ExitApp, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering ExitApp callback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterPauseCallback(PauseApp, &g_pause_call_counter);
    if (result != gfnSuccess)
    {
        printf("Error registering PauseApp callback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterSaveCallback(AutoSave, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering AutoSave callback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterSessionInitCallback(SessionInit, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering SessionInit callback:%d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterMessageCallback(MessageCallback, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering MessageCallback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterClientInfoCallback(HandleClientDataChanges, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering clientInfo callback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    result = GfnRegisterNetworkStatusCallback(HandleNetworkStatusChanges, 10000 /* 10 seconds */, NULL);
    if (result != gfnSuccess)
    {
        printf("Error registering clientInfo callback: %d, %s\n", result, GfnErrorToString(result));
        overallResult = result;
    }
    return overallResult;
}

// Example application shutdown method with a call to shut down the GeForce NOW Runtime SDK
void ApplicationShutdown()
{
    printf("\n\nApplication: Shutting down...\n");

    // Shut down the GeForce NOW Runtime SDK. Note that it's safe to call
    // gfnShutdownRuntimeSdk even if the SDK was not initialized.
    GfnShutdownSdk();
}

// Example application main
int main()
{
    GfnError result = gfnSuccess;
    result = ApplicationInitialize();
    if (GFNSDK_FAILED(result))
    {
        printf("Initialization failed, sample will now exit");
        WaitForSpaceBar();
        return -1;
    }

    // Simple Cloud Check API call for non-critical decision-making 
    bool bIsCloudEnvironment = false;
    result = GfnIsRunningInCloud(&bIsCloudEnvironment);
    if (!bIsCloudEnvironment || GFNSDK_FAILED(result))
    {
        printf("GfnIsRunningInCloud: Application is executing on a local client, outside of the GeForce NOW cloud environment.\n");
        printf("No cloud-based SDK APIs will be called. For information on how to test in GeForce NOW, please visit https://developer.geforcenow.com/.\n");
        WaitForSpaceBar();
        ApplicationShutdown();
        return -1;
    }
    
    // Running inside GeForce NOW, the callbacks can be registered
    result = RegisterCallbacks();
    if (GFNSDK_FAILED(result))
    {
        printf("RegisterCallbacks: At least one failure registering for SDK callback notifications.\n");
    }

    //CloudCheck API call with response validation for more critical decision-making scenarios
    // Generate a nonce, the minimum nonce size requirement is 16 bytes.
    char nonce[CLOUD_CHECK_MIN_NONCE_SIZE] = { 0 };
    if (!GfnCloudCheckGenerateNonce(nonce, CLOUD_CHECK_MIN_NONCE_SIZE))
    {
        printf("GenerateNonce Failed\n");
    }
    else
    {
        GfnCloudCheckChallenge challenge = { nonce, CLOUD_CHECK_MIN_NONCE_SIZE };
        GfnCloudCheckResponse response = { NULL, 0 };
        result = GfnCloudCheck(&challenge, &response, &bIsCloudEnvironment);
        if (result == gfnSuccess)
        {
            printf("\nGfnCloudCheck: Application executing in GeForce NOW environment: %s\n", (bIsCloudEnvironment == true) ? "true" : "false");
            if (response.attestationData != NULL)
            {
                if (GfnCloudCheckVerifyAttestationData(response.attestationData, challenge.nonce, challenge.nonceSize))
                {
                    printf("CloudCheck response validated\n");
                }
                else
                {
                    printf("CloudCheck response validation failed\n\n");
                }
                GfnFree(&response.attestationData);
            }
        }
        else
        {
            if (result == gfnThrottled)
            {
                printf("GfnCloudCheck: API call rate limit exceeded\n");
            }
            else
            {
                printf("GfnCloudCheck: Failed to check if running in GeForce NOW Cloud environment: %d, %s\n", result, GfnErrorToString(result));
            }
        }
    }
    if (bIsCloudEnvironment) // More sample C API calls.
    {

        char const* clientIp = NULL;
        result = GfnGetClientIpV4(&clientIp);
        if (result == gfnSuccess)
        {
            printf("Retrieved GeForce NOW Client I.P.: %s\n", clientIp);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW Client I.P. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }

        char const* clientLanguageCode = NULL;
        result = GfnGetClientLanguageCode(&clientLanguageCode);
        if (result == gfnSuccess)
        {
            printf("Retrieved GeForce NOW client language code: %s\n", clientLanguageCode);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW client language code. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }

        char clientCountryCode[3];
        result = GfnGetClientCountryCode(clientCountryCode, 3);
        if (result == gfnSuccess)
        {
            printf("Retrieved GeForce NOW client Country code: %s\n", clientCountryCode);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW client Country code. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }

        GfnRect rect = { 0.5f, 0.5f, 0.1f, 0.1f, true, gfnRectXYWH };
        GfnSetEditBoxRegion(1, &rect, 0);
        rect.value1 = rect.value2 = 0.1f;
        GfnSetEditBoxRegion(2, &rect, GFN_EDITBOX_FLAG_LONG_PRESS_OVERRIDE);
        GfnClearEditBoxRegion(1);
        rect.value1 = rect.value2 = 0.2f;
        GfnSetEditBoxRegion(3, &rect, GFN_EDITBOX_FLAG_LONG_PRESS_OVERRIDE);
        GfnClearEditBoxRegion(2);
        GfnClearEditBoxRegion(3);
        result = GfnSetActionZone(gfnEditBox, 1, &rect);
        if (result == gfnSuccess)
        {
            printf("Set an action zone.\n");
        }
        else
        {
            printf("Failed to set an action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        rect.value1 = rect.value2 = 0.1f;
        result = GfnSetActionZone(gfnEditBox, 2, &rect);
        if (result == gfnSuccess)
        {
            printf("Set a second action zone.\n");
        }
        else
        {
            printf("Failed to set a second action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        result = GfnSetActionZone(gfnEditBox, 1, NULL);
        if (result == gfnSuccess)
        {
            printf("Cleared the first action zone.\n");
        }
        else
        {
            printf("Failed to clear the first action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        rect.value1 = rect.value2 = 0.2f;
        result = GfnSetActionZone(gfnEditBox, 3, &rect);
        if (result == gfnSuccess)
        {
            printf("Set a third action zone.\n");
        }
        else
        {
            printf("Failed to set a third action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        result = GfnSetActionZone(gfnEditBox, 2, NULL);
        if (result == gfnSuccess)
        {
            printf("Cleared the second action zone.\n");
        }
        else
        {
            printf("Failed clear the second action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        result = GfnSetActionZone(gfnEditBox, 3, NULL);
        if (result == gfnSuccess)
        {
            printf("Cleared the third action zone.\n");
        }
        else
        {
            printf("Failed clear the third action zone. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }

        char const* partnerSecureData = NULL;
        result = GfnGetPartnerSecureData(&partnerSecureData);
        if (GFNSDK_SUCCEEDED(result)) {
            printf("GfnGetPartnerSecureData: [%s]\n", partnerSecureData);
            GfnFree(&partnerSecureData);
        }
        else
        {
            printf("Failed to retrieve PartnerSecureData\n");
        }

        GfnClientInfo clientInfo = { 0 };
        result = GfnGetClientInfo(&clientInfo);
        if (result == gfnSuccess)
        {
            printf("GetClientInfo returned: { version: %d, osType: %d, ipV4: %s"
                ", country: %s, locale:%s"
                ", RTDAverageLatencyMs: %d"
                ", clientResolution: %dx%d"

                " }\n",
                clientInfo.version, clientInfo.osType, clientInfo.ipV4,
                clientInfo.country, clientInfo.locale
                , clientInfo.RTDAverageLatencyMs
                , clientInfo.clientResolution.horizontalPixels, clientInfo.clientResolution.verticalPixels
            );
        }
        else
        {
            printf("Failed to retrieve client info. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
        GfnSessionInfo sessionInfo = { 0 };
        result = GfnGetSessionInfo(&sessionInfo);
        if (result == gfnSuccess)
        {
            printf("Initial GetSessionInfo returned: { session length: %d, session remaining: %d}\n",
                sessionInfo.sessionMaxDurationSec, sessionInfo.sessionTimeRemainingSec
            );
        }
        else
        {
            printf("Failed to retrieve initial session info. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
    }

    // Application main loop
    WaitForSpaceBar();

    if (bIsCloudEnvironment)
    {
        // Call GetSessionInfo again to confirm less time remaining than before
        GfnSessionInfo sessionInfo = { 0 };
        result = GfnGetSessionInfo(&sessionInfo);
        if (result == gfnSuccess)
        {
            printf("Final GetSessionInfo returned: { session length: %d, session remaining: %d}\n",
                sessionInfo.sessionMaxDurationSec, sessionInfo.sessionTimeRemainingSec
            );
        }
        else
        {
            printf("Failed to retrieve final session info. GfnError: %d, %s\n", result, GfnErrorToString(result));
        }
    }

    // Application Shutdown
    // It's safe to call ShutdownShieldXLinkSDK even if the SDK was not initialized.
    ApplicationShutdown();

    return 0;
}
