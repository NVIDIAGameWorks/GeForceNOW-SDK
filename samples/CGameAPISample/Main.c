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
const char* platformAppId = "GeForce NOW SDK - Sample C App";
const char* platformId = "GeForce NOW SDK - Sample Test Platform";
#define CLOUD_CHECK_MIN_NONCE_SIZE 16

static char getKeyPress() {
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

static void waitForSpaceBar() {
    printf("\n\nApplication: In main application loop; Press space bar to exit...\n\n");
    char c;
    do
    {
        c = getKeyPress();
    } while (c != ' ');
}

// Example application initialization method with a call to initialize the GeForce NOW Runtime SDK.
// Application callbacks are registered with the SDK after it is initialized if running in Cloud mode.
void ApplicationInitialize()
{
    printf("\n\nApplication: Initializing...\n");

    // Initialize the GeForce NOW Runtime SDK using the C calling convention.
    GfnRuntimeError err = GfnInitializeSdk(gfnDefaultLanguage);
    if (GFNSDK_FAILED(err))
    {
        // Initialization errors generally indicate a flawed environment. Check error code for details.
        // See GfnError in GfnSdk.h for error codes.
        printf("Error initializing the SDK: %d, %s\n", err, GfnErrorToString(err));
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
                printf("Error registering ExitApp callback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterPauseCallback(PauseApp, &g_pause_call_counter);
            if (err != gfnSuccess)
            {
                printf("Error registering PauseApp callback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterInstallCallback(InstallApp, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering InstallApp callback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterSaveCallback(AutoSave, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering AutoSave callback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterSessionInitCallback(SessionInit, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering SessionInit callback:%d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterMessageCallback(MessageCallback, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering MessageCallback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterClientInfoCallback(HandleClientDataChanges, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering clientInfo callback: %d, %s\n", err, GfnErrorToString(err));
            }
            err = GfnRegisterNetworkStatusCallback(HandleNetworkStatusChanges, 10000 /* 10 seconds */, NULL);
            if (err != gfnSuccess)
            {
                printf("Error registering clientInfo callback: %d, %s\n", err, GfnErrorToString(err));
            }
        }
    }

    // Application Initialization here
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
    GfnError err = gfnSuccess;

    ApplicationInitialize();

    // Simple Cloud Check API call
    bool bIsCloudEnvironment = false;
    GfnIsRunningInCloud(&bIsCloudEnvironment);
    printf("\nGfnIsRunningInCloud: Application executing in GeForce NOW environment: %s\n", (bIsCloudEnvironment == true) ? "true" : "false");

    //CloudCheck API call with response validation
    bool bCloudCheck = false;

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
        err = GfnCloudCheck(&challenge, &response, &bCloudCheck);
        if (err == gfnSuccess)
        {
            printf("\nGfnCloudCheck: Application executing in GeForce NOW environment: %s\n", (bCloudCheck == true) ? "true" : "false");
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
            if (err == gfnThrottled)
            {
                printf("GfnCloudCheck: API call rate limit exceeded\n");
            }
            else
            {
                printf("GfnCloudCheck: Failed to check if running in GeForce NOW Cloud environment: %d, %s\n", err, GfnErrorToString(err));
            }
        }
    }
    if (bCloudCheck) // More sample C API calls.
    {
        char const* clientIp = NULL;
        err = GfnGetClientIpV4(&clientIp);
        if (err == gfnSuccess)
        {
            printf("Retrieved GeForce NOW Client I.P.: %s\n", clientIp);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW Client I.P. GfnError: %d, %s\n", err, GfnErrorToString(err));
        }

        char const* clientLanguageCode = NULL;
        err = GfnGetClientLanguageCode(&clientLanguageCode);
        if (err == gfnSuccess)
        {
            printf("Retrieved GeForce NOW client language code: %s\n", clientLanguageCode);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW client language code. GfnError: %d, %s\n", err, GfnErrorToString(err));
        }

        char clientCountryCode[3];
        err = GfnGetClientCountryCode(clientCountryCode, 3);
        if (err == gfnSuccess)
        {
            printf("Retrieved GeForce NOW client Country code: %s\n", clientCountryCode);
        }
        else
        {
            printf("Failed to retrieve GeForce NOW client Country code. GfnError: %d, %s\n", err, GfnErrorToString(err));
        }
        char const* partnerSecureData = NULL;
        err = GfnGetPartnerSecureData(&partnerSecureData);
        if (GFNSDK_SUCCEEDED(err)) {
            printf("GfnGetPartnerSecureData: [%s]\n", partnerSecureData);
            GfnFree(&partnerSecureData);
        }
        else {
            printf("Failed to retrieve PartnerSecureData\n");
        }

        char const* partnerData = NULL;
        err = GfnGetPartnerData(&partnerData);
        if (GFNSDK_SUCCEEDED(err)) {
            printf("GfnGetPartnerData: [%s]\n", partnerData);
            GfnFree(&partnerData);
        }
        else {
            printf("Failed to retrieve PartnerData\n");
        }

        GfnClientInfo clientInfo = { 0 };
        err = GfnGetClientInfo(&clientInfo);
        if (err == gfnSuccess)
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
            printf("Failed to retrieve client info. GfnError: %d, %s\n", err, GfnErrorToString(err));
        }

        GfnSessionInfo sessionInfo = { 0 };
        err = GfnGetSessionInfo(&sessionInfo);
        if (err == gfnSuccess)
        {
            printf("Initial GetSessionInfo returned: { session length: %d, session remaining: %d}\n",
                sessionInfo.sessionMaxDurationSec, sessionInfo.sessionTimeRemainingSec
            );
        }
        else
        {
            printf("Failed to retrieve initial session info. GfnError: %d, %s\n", err, GfnErrorToString(err));
        }

        // Try "setting up" a title!
        err = GfnSetupTitle(platformAppId);
        if (err == gfnSuccess)
        {
            printf("Setup GFN Title\n");
        }
        else
        {
            printf("Failed to setup GFN Title: %d, %s\n", err, GfnErrorToString(err));
        }
    }

    // Application main loop
    waitForSpaceBar();

    // Notify Title exited
    err = GfnTitleExited(platformId, platformAppId);
    if (err == gfnSuccess)
    {
        printf("GFN Title Exited\n");
    }
    else
    {
        printf("GFN Title Exited failed: %d, %s\n", err, GfnErrorToString(err));
    }

    // Call GetSessionInfo again to confirm less time remaining than before
    GfnSessionInfo sessionInfo = { 0 };
    err = GfnGetSessionInfo(&sessionInfo);
    if (err == gfnSuccess)
    {
        printf("Final GetSessionInfo returned: { session length: %d, session remaining: %d}\n",
            sessionInfo.sessionMaxDurationSec, sessionInfo.sessionTimeRemainingSec
        );
    }
    else
    {
        printf("Failed to retrieve final session info. GfnError: %d, %s\n", err, GfnErrorToString(err));
    }

    // Application Shutdown
    // It's safe to call ShutdownShieldXLinkSDK even if the SDK was not initialized.
    ApplicationShutdown();

    return 0;
}
