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
#include <string.h>
#include "GfnRuntimeSdk_CAPI.h"

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
    return crCallbackSuccess;
}

GfnApplicationCallbackResult GFN_CALLBACK HandleClientDataChanges(GfnClientInfoUpdateData* pUpdate, const void* pContext)
{
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
    default:
        printf("Unknown client data change type %d\n", pUpdate->updateType);
        break;
    }
    return crCallbackSuccess;
}
