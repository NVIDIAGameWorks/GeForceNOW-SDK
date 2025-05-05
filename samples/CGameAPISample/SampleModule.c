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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GfnRuntimeSdk_Wrapper.h"

extern bool g_MainDone;

GfnApplicationCallbackResult GFN_CALLBACK ExitApp(void* pContext)
{
    // Callback for when GeForce NOW will begin to shutdown the session.
    // This normally occurs when the user choses to close the streaming window instead of the title
    // being streamed to the window.
    // Applications responding to this callback should trigger their shutdown activities.
    printf("Received Exit callback. Application cleaning up and exiting, application context: %p\n", pContext);
    g_MainDone = true;
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK PauseApp(void* pContext)
{
    // Callback for when GeForce NOW requests the application to pause.
    // This normally occurs when the user changes focus away from the streaming window.
    // Applications should enter their paused state, and wait for direct input from user to resume.
    // For the sake of example, imagine we treat the context like an int and print
    // the number of times an application pause has been requested by the SDK.
    printf("Application paused (%d call(s)).\n", *((int*) pContext) + 1);
    *((int*) pContext) += 1;
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK InstallApp(TitleInstallationInformation* pInfo, void* pContext)
{
    // Callback for when GeForce NOW finished installing the requested application.
    // Any last install configuration steps should be performed at this time.
    printf("\"Completing\" title setup for: %s.\n\tBuild path: %s\n\tMetadata path: %s\n\tApplication Context: %p\n",
        pInfo->pchPlatformAppId, pInfo->pchBuildPath, pInfo->pchMetadataPath, pContext);
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK AutoSave(void* pContext)
{
    // Callback for when GeForce NOW requests the application to save its state, including user data.
    // This can occur during shutdown, which GFN needs the save data to save as part of the user's account state
    // so they do not lose progress for the next GeForce NOW streaming session.
    printf("AutoSave triggered, application context: %p\n", pContext);
    return crCallbackSuccess;
}


GfnApplicationCallbackResult GFN_CALLBACK SessionInit(const char* params, void* pContext)
{
    // Callback for when GeForce NOW a user connects to the game seat to start a streaming session.
    // This is used in Pre-Warm launch flows to alert a pre-launched title to load user data for the
    // connected user and start a streaming session.
    // Respond within 30 seconds with a call to gfnAppReady API
    printf("SessionInit: %s\nApplication context: %p\n", params, pContext);
    // Report that the application is ready for streaming to begin
    GfnError runtimeError = GfnAppReady(true, "Abcd");
    if (runtimeError == gfnSuccess)
    {
        printf("Reported 'AppReady' with success to the SDK\n");
    }
    else
    {
        printf("Failed to report 'AppReady' to the SDK: %d\n", (int)runtimeError);
    }

    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK MessageCallback(const GfnString* pMessage, void* pContext)
{
    printf("Message from client: %s length=%u\n", pMessage->pchString, pMessage->length);
    printf("Application Context %p\n", pContext);

    // Send a message back to the client in response
    unsigned int outLength = pMessage->length + 6;

    char* pchTestMessage = malloc(sizeof *pchTestMessage * outLength);
    outLength = snprintf(pchTestMessage, outLength, "ACK: %s", pMessage->pchString);

    GfnError runtimeError = GfnSendMessage(pchTestMessage, outLength);
    if (runtimeError == gfnSuccess)
    {
        printf("Sent a communication message to the client: %s\n", pchTestMessage);
    }
    else
    {
        printf("Failed to send a communication message to the client. GfnError: % d\n", (int)runtimeError);
    }

    free(pchTestMessage);
    pchTestMessage = NULL;

    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK HandleClientDataChanges(GfnClientInfoUpdateData* pUpdate, const void* pContext)
{
    // Callback for when GeForce NOW detects client-sourced data changes.
    // This can occur when a user rotates their mobile device or resumes the session on a different device.
    printf("HandleClientDataChanges::Application context: %p\n", pContext);
    if (!pUpdate)
    {
        printf("Client info callback received invalid data\n");
        return crCallbackFailure;
    }

    switch (pUpdate->updateType)
    {
    case gfnOs:
        printf("OS changed: %d\n", pUpdate->data.osType);
        break;
    case gfnIP:
        printf("IP changed: %s\n", pUpdate->data.ipV4);
        break;
    case gfnClientResolution:
        printf("Resolution changed: %dx%d\n", pUpdate->data.clientResolution.horizontalPixels, pUpdate->data.clientResolution.verticalPixels);
        break;
    case gfnSafeZone:
        // Safe zone information always comes in as normalized gfnRectLTRB formatted values.
        // See Mobile Integration Guide for more information.
        printf("SafeZone changed: %2.2f, %2.2f, %2.2f, %2.2f\n", pUpdate->data.safeZone.value1, pUpdate->data.safeZone.value2, pUpdate->data.safeZone.value3, pUpdate->data.safeZone.value4);
        break;
    default:
        printf("Unknown client data change type %d\n", pUpdate->updateType);
        break;
    }
    return crCallbackSuccess;
}
GfnApplicationCallbackResult GFN_CALLBACK HandleNetworkStatusChanges(GfnNetworkStatusUpdateData* pUpdate, const void* pContext)
{
    // Callback for when GeForce NOW detects changes to network latency.
    // Use the new value to decide latency related decisions, such as input delay or matchmaking.
    printf("HandleNetworkStatusChanges::Application context: %p\n", pContext);
    if (!pUpdate)
    {
        printf("Network perf callback received invalid data\n");
        return crCallbackFailure;
    }
    if (pUpdate->updateType == gfnRTDAverageLatency)
    {
        printf("RTD data received %d\n", pUpdate->data.RTDAverageLatencyMs);
    }
    return crCallbackSuccess;
}
