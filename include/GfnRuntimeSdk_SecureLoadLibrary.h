// ===============================================================================================
//
/* Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of NVIDIA CORPORATION nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__
#define __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__
#include <Shlobj.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// @par Description
/// Provides a more secure version of Win32 LoadLibrary API with NVIDIA digital signature validation
///
/// @par Environment
/// Cloud and Client 
///
/// @par Usage
/// Use to securely load the NVIDIA signed libraries, such as GeForce NOW SDK dynamic library
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
HMODULE gfnSecureLoadLibraryExW(LPCWSTR filePath, DWORD dwFlags);
HMODULE gfnSecureLoadLibraryExA(LPCSTR filePath, DWORD dwFlags);

#ifdef UNICODE
#define gfnSecureLoadLibraryEx gfnSecureLoadLibraryExW
#define gfnSecureLoadLibrary gfnSecureLoadLibraryW
#else
#define gfnSecureLoadLibraryEx gfnSecureLoadLibraryExA
#define gfnSecureLoadLibrary gfnSecureLoadLibraryA
#endif /* !UNICODE */

#ifdef __cplusplus
}
#endif

#endif __NV_GFNSDK_SECURE_LOAD_LIBRARY_H__