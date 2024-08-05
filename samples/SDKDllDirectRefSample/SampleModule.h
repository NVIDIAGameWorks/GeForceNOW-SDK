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
#include "GfnRuntimeSdk_CAPI.h"

#ifdef _WIN32
#include <windows.h>    // For GetAsyncKeyState
typedef HMODULE HANDLE_TYPE;
#elif defined(__linux__)
#include <dlfcn.h>      // dlsym
typedef void* HANDLE_TYPE;
#else
#error "Unsupported platform"
#endif

extern HANDLE_TYPE gfnSdkModule;

static void* GetSymbol(HANDLE_TYPE library, char* name)
{
#ifdef _WIN32
    return (void*)GetProcAddress(library, name);
#elif defined(__linux__)
    return dlsym(library, name);
#endif
}

extern GfnApplicationCallbackResult GFN_CALLBACK ExitApp(void* pContext);

extern GfnApplicationCallbackResult GFN_CALLBACK PauseApp(void* pContext);

extern GfnApplicationCallbackResult GFN_CALLBACK InstallApp(TitleInstallationInformation* pInfo, void* pContext);

extern GfnApplicationCallbackResult GFN_CALLBACK AutoSave(void* pContext);


extern GfnApplicationCallbackResult GFN_CALLBACK SessionInit(const char* params, void* pContext);

extern GfnApplicationCallbackResult GFN_CALLBACK HandleClientDataChanges(GfnClientInfoUpdateData* data, const void* pContext);

extern GfnApplicationCallbackResult GFN_CALLBACK HandleNetworkStatusChanges(GfnNetworkStatusUpdateData* pUpdate, const void* pContext);
