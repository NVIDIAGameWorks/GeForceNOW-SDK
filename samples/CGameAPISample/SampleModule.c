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
    printf("Application exiting...\n");
    g_MainDone = true;
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK PauseApp(void* pContext)
{
    // For the sake of example, imagine we treat the context like an int and print
    // the number of times an application pause has been requested by the SDK.
    printf("Application paused (%d call(s)).\n", *((int*) pContext) + 1);
    *((int*) pContext) += 1;
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK InstallApp(TitleInstallationInformation* pInfo, void* pContext)
{
    printf("\"Completing\" title setup for: %s.\n\tBuild path: %s\n\tMetadata path: %s\n",
        pInfo->pchPlatformAppId, pInfo->pchBuildPath, pInfo->pchMetadataPath);
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK AutoSave(void* pContext)
{
    printf("AutoSave\n");
    return crCallbackSuccess;
}


GfnApplicationCallbackResult GFN_CALLBACK SessionInit(const char* params, void* pContext)
{
    printf("SessionInit: %s\n", params);

    // Report that the application is ready for streaming to begin
    GfnError runtimeError = GfnAppReady(false, "Abcd");
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

    // Send a message back to the client in response
    const char* pchFormat = "ACK: %s";

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
    if (!pUpdate)
    {
        printf("Client info callback received invalid data\n");
        return crCallbackFailure;
    }

    printf("sizeof GfnClientInfoUpdateData: %d\n", (int)(sizeof(*pUpdate)));

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
        // Safe zone information always comes in as normalized gfnRectLTRB formatted values
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
