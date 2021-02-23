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
// Copyright (c) 2020-2021 NVIDIA Corporation. All rights reserved.

#ifndef __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__
#define __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

///
/// @par Description
/// Provides a more secure version of Win32 LoadLibrary API with NVIDIA digital signature validation
///
/// @par Environment
/// Client
///
/// @par Usage
/// Use to securely load the NVIDIA signed libraries, such as the distributed GfnRuntime Dll
///
/// @param filePath                 - Fully quantified path to the library to load.
///                                   Use backslash ("\") for path separators.
///                                   See notes section for additional information.
/// @param dwFlags                  - Optional flags passed to LoadLibrary
///
///
/// @note
/// In case of failure, call GetLastError for additional information about the error. Common error
/// values are:
/// - ERROR_SUCCESS                 - no error, the library was loaded
/// - ERROR_BAD_ARGUMENTS           - filePath is not absolute
/// - ERROR_INVALID_PARAMETER       - parameter passed is NULL
/// - ERROR_MOD_NOT_FOUND           - file / module not found
/// - CRYPT_E_NO_MATCH              - no certificate found, or certificate wasn't issued by NVIDIA
///
HMODULE gfnSecureLoadClientLibraryW(LPCWSTR filePath, DWORD dwFlags);
HMODULE gfnSecureLoadClientLibraryA(LPCSTR filePath, DWORD dwFlags);

///
/// @par Description
/// Provides a more secure version of Win32 LoadLibrary API with NVIDIA digital signature validation
///
/// @par Environment
/// Cloud
///
/// @par Usage
/// Use to securely load the NVIDIA signed libraries provided by the GeForceNOW cloud game seat
///
/// @param filePath                 - Fully quantified path to the library to load.
///                                   Use backslash ("\") for path separators.
///                                   See notes section for additional information.
/// @param dwFlags                  - Optional flags passed to LoadLibrary
///
///
/// @note
/// In case of failure, call GetLastError for additional information about the error. Common error
/// values are:
/// - ERROR_SUCCESS                 - no error, the library was loaded
/// - ERROR_BAD_ARGUMENTS           - filePath is not absolute
/// - ERROR_INVALID_PARAMETER       - parameter passed is NULL
/// - ERROR_MOD_NOT_FOUND           - file / module not found
/// - CRYPT_E_NO_MATCH              - no certificate found, or certificate wasn't issued by NVIDIA
///
HMODULE gfnSecureLoadCloudLibraryW(LPCWSTR filePath, DWORD dwFlags);
HMODULE gfnSecureLoadCloudLibraryA(LPCSTR filePath, DWORD dwFlags);

#ifdef UNICODE
#define gfnSecureLoadClientLibrary gfnSecureLoadClientLibraryW
#define gfnSecureLoadCloudLibrary gfnSecureLoadCloudLibraryW
#else
#define gfnSecureLoadClientLibrary gfnSecureLoadClientLibraryA
#define gfnSecureLoadCloudLibrary gfnSecureLoadCloudLibraryA
#endif /* !UNICODE */

#ifdef __cplusplus
}
#endif

#endif __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__