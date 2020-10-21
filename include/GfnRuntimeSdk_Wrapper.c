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


#include "GfnRuntimeSdk_Wrapper.h"

#ifdef _WIN32
HMODULE g_gfnSdkModule;

// Library export function definitions
typedef GfnRuntimeError(*gfnInitializeRuntimeSdkFn)(GfnDisplayLanguage language);
typedef void(*gfnShutdownRuntimeSdkFn)(void);
typedef bool(*gfnIsRunningInCloudFn)(void);
typedef GfnRuntimeError(*gfnGetClientIpFn)(const char** cientIp);
typedef GfnRuntimeError(*gfnGetClientLanguageCodeFn)(const char** clientLanguageCode);
typedef GfnRuntimeError(*gfnGetClientCountryCodeFn)(char* clientCountryCode, unsigned int length);
typedef GfnRuntimeError(*gfnGetCustomDataFn)(const char** customData);
typedef GfnRuntimeError(*gfnGetAuthDataFn)(const char** authData);

typedef bool (*gfnIsTitleAvailableFn)(const char* platformAppId);
typedef GfnRuntimeError(*gfnGetTitlesAvailableFn)(const char** platformAppIds);

typedef GfnRuntimeError(*gfnFreeFn)(const char** data);

typedef GfnRuntimeError(*gfnRegisterStreamStatusCallbackFn)(StreamStatusCallbackSig streamStatusCallback, void* pUserContext);
typedef GfnRuntimeError(*gfnStartStreamFn)(StartStreamInput * startStreamInput, StartStreamResponse* response);
typedef void(*gfnStartStreamAsyncFn)(const StartStreamInput* startStreamInput, StartStreamCallbackSig cb, void* context, unsigned int timeoutMs);

typedef GfnRuntimeError(*gfnStopStreamFn)(void);
typedef void(*gfnStopStreamAsyncFn)(StopStreamCallbackSig cb, void* context, unsigned int timeoutMs);

typedef GfnRuntimeError(*gfnSetupTitleFn)(const char* platformAppId);
typedef GfnRuntimeError(*gfnTitleExitedFn)(const char* platformId, const char* platformAppId);
typedef GfnRuntimeError(*gfnRegisterExitCallbackFn)(ExitCallbackSig exitCallback, void* userContext);
typedef GfnRuntimeError(*gfnRegisterSaveCallbackFn)(SaveCallbackSig exitCallback, void* userContext);
typedef GfnRuntimeError(*gfnRegisterPauseCallbackFn)(PauseCallbackSig exitCallback, void* userContext);
typedef GfnRuntimeError(*gfnRegisterInstallCallbackFn)(InstallCallbackSig exitCallback, void* userContext);

GfnRuntimeError GfnInitializeSdk(GfnDisplayLanguage language)
{
    if (g_gfnSdkModule != NULL)
    {
        // Already initialized, no need to initialize again
        return gfnSuccess;
    }
    
    wchar_t * filename = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);
    DWORD pathSize = GetModuleFileNameW(NULL, filename, MAX_PATH);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        free(filename);
        filename = (wchar_t*)malloc(sizeof(wchar_t) * (pathSize + 1)); //Room for \0
        pathSize = GetModuleFileNameW(NULL, filename, pathSize + 1);
    }

    wchar_t* lastSepPos = wcsrchr(filename, L'\\');
    size_t pathLength = lastSepPos - filename + 1;
    size_t dllPathLength = pathLength + 18; // "GfnRuntimeSdk.dll\0"
    wchar_t* dllPath = (wchar_t*)malloc(sizeof(wchar_t) * dllPathLength);
    wcsncpy_s(dllPath, dllPathLength, filename, pathLength);
    dllPath[pathLength] = L'\0';
    free(filename);
    wcsncat_s(dllPath, dllPathLength, L"GfnRuntimeSdk.dll", 17);

    // For security reasons, it is preferred to check the digital signature before loading the DLL.
    // Such code is not provided here to reduce code complexity and library size, and in favor of
    // any internal libraries built for this purpose.
    g_gfnSdkModule = LoadLibraryW(dllPath);
    free(dllPath);
    if (g_gfnSdkModule == NULL)
    {
        return gfnDllNotPresent;
    }
    gfnInitializeRuntimeSdkFn fnGfnInitializeRuntimeSdk = (gfnInitializeRuntimeSdkFn)GetProcAddress(g_gfnSdkModule, "gfnInitializeRuntimeSdk");
    if (fnGfnInitializeRuntimeSdk == NULL)
    {
        return gfnAPINotFound;
    }

    return (fnGfnInitializeRuntimeSdk)(language);
}

GfnRuntimeError GfnShutdownSdk(void)
{
    if (g_gfnSdkModule == NULL)
    {
        // Not initialized, no need to shutdown
        return gfnSuccess;
    }

    gfnShutdownRuntimeSdkFn fnGfnShutdownRuntimeSdk = (gfnShutdownRuntimeSdkFn)GetProcAddress(g_gfnSdkModule, "gfnShutdownRuntimeSdk");
    if (fnGfnShutdownRuntimeSdk == NULL)
    {
        return gfnAPINotFound;
    }

    fnGfnShutdownRuntimeSdk();
    return gfnSuccess;
}

GfnRuntimeError GfnIsRunningInCloud(bool* runningInCloud)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnIsRunningInCloudFn fnGfnIsRunningInCloud = (gfnIsRunningInCloudFn)GetProcAddress(g_gfnSdkModule, "gfnIsRunningInCloud");
    if (fnGfnIsRunningInCloud == NULL)
    {
        return gfnAPINotFound;
    }

    *runningInCloud = fnGfnIsRunningInCloud();
    return gfnSuccess;
}

GfnRuntimeError GfnFree(const char** data)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    if (!data)
    {
        return gfnSuccess;
    }

    gfnFreeFn fnGfnFree = (gfnFreeFn)GetProcAddress(g_gfnSdkModule, "gfnFree");

    if (fnGfnFree == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnFree(data);
}

GfnRuntimeError GfnGetClientIpV4(const char ** clientIp)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    // API sets the char* to a static-declared string, which means it is not necessary to allocate/free memory
    gfnGetClientIpFn fnGfnGetClientIp = (gfnGetClientIpFn)GetProcAddress(g_gfnSdkModule, "gfnGetClientIp");

    if (fnGfnGetClientIp == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetClientIp(clientIp);
}

GfnRuntimeError GfnGetClientLanguageCode(const char** languageCode)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    // API sets the char* to a static-declared string, which means it is not necessary to allocate/free memory
    gfnGetClientLanguageCodeFn fnGfnGetClientLanguageCode = (gfnGetClientLanguageCodeFn)GetProcAddress(g_gfnSdkModule, "gfnGetClientLanguageCode");

    if (fnGfnGetClientLanguageCode == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetClientLanguageCode(languageCode);
}

GfnRuntimeError GfnGetClientCountryCode(char* countryCode, unsigned int length)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnGetClientCountryCodeFn fnGfnGetClientCountryCode = (gfnGetClientCountryCodeFn)GetProcAddress(g_gfnSdkModule, "gfnGetClientCountryCode");

    if (fnGfnGetClientCountryCode == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetClientCountryCode(countryCode, length);
}

GfnRuntimeError GfnGetCustomData(const char** customData)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    // API sets the char* to a static-declared string, which means it is not necessary to allocate/free memory
    gfnGetCustomDataFn fnGfnGetCustomData = (gfnGetCustomDataFn)GetProcAddress(g_gfnSdkModule, "gfnGetCustomData");

    if (fnGfnGetCustomData == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetCustomData(customData);
}

GfnRuntimeError GfnGetAuthData(const char** authData)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnGetAuthDataFn fnGfnGetAuthData = (gfnGetAuthDataFn)GetProcAddress(g_gfnSdkModule, "gfnGetAuthData");

    if (fnGfnGetAuthData == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetAuthData(authData);
}

GfnRuntimeError GfnIsTitleAvailable(const char* platformAppId, bool* isAvailable)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    if (!platformAppId)
    {
        return gfnInvalidParameter;
    }

    gfnIsTitleAvailableFn fnGfnIsTitleAvailable = (gfnIsTitleAvailableFn)GetProcAddress(g_gfnSdkModule, "gfnIsTitleAvailable");

    if (fnGfnIsTitleAvailable == NULL)
    {
        return gfnAPINotFound;
    }

    *isAvailable = fnGfnIsTitleAvailable(platformAppId);

    return gfnSuccess;
}

GfnRuntimeError GfnGetTitlesAvailable(const char** platformAppIds)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    if (!platformAppIds)
    {
        return gfnInvalidParameter;
    }

    gfnGetTitlesAvailableFn fnGfnGetTitlesAvailable = (gfnGetTitlesAvailableFn)GetProcAddress(g_gfnSdkModule, "gfnGetTitlesAvailable");

    if (fnGfnGetTitlesAvailable == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnGetTitlesAvailable(platformAppIds);
}

GfnRuntimeError GfnRegisterStreamStatusCallback(StreamStatusCallbackSig streamStatusCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterStreamStatusCallbackFn fnRegisterStreamStatusCallback = (gfnRegisterStreamStatusCallbackFn)GetProcAddress(g_gfnSdkModule, "gfnRegisterStreamStatusCallback");

    if (fnRegisterStreamStatusCallback == NULL)
    {
        return gfnAPINotFound;
    }

    return fnRegisterStreamStatusCallback(streamStatusCallback, userContext);
}

GfnRuntimeError GfnStartStream(StartStreamInput * startStreamInput, StartStreamResponse* response)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnStartStreamFn fnGfnStartStream = (gfnStartStreamFn)GetProcAddress(g_gfnSdkModule, "gfnStartStream");

    if (fnGfnStartStream == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnStartStream(startStreamInput, response);
}

GfnRuntimeError GfnStartStreamAsync(const StartStreamInput* startStreamInput, StartStreamCallbackSig cb, void* context, unsigned int timeoutMs)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnStartStreamAsyncFn fnGfnStartStreamAsync = (gfnStartStreamAsyncFn)GetProcAddress(g_gfnSdkModule, "gfnStartStreamAsync");

    if (fnGfnStartStreamAsync == NULL)
    {
        return gfnAPINotFound;
    }

    fnGfnStartStreamAsync(startStreamInput, cb, context, timeoutMs);

    return gfnSuccess;
}

GfnRuntimeError GfnStopStream(void)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnStopStreamFn fnGfnStopStream = (gfnStopStreamFn)GetProcAddress(g_gfnSdkModule, "gfnStopStream");

    if (fnGfnStopStream == NULL)
    {
        return gfnAPINotFound;
    }

    return fnGfnStopStream();
}

GfnRuntimeError GfnStopStreamAsync(StopStreamCallbackSig cb, void* context, unsigned int timeoutMs)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnStopStreamAsyncFn fnGfnStopStreamAsync = (gfnStopStreamAsyncFn)GetProcAddress(g_gfnSdkModule, "gfnStopStreamAsync");

    if (fnGfnStopStreamAsync == NULL)
    {
        return gfnAPINotFound;
    }

    fnGfnStopStreamAsync(cb, context, timeoutMs);

    return gfnSuccess;
}

GfnRuntimeError GfnSetupTitle(const char* platformAppId)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnSetupTitleFn fnSetupTitle= (gfnSetupTitleFn)GetProcAddress(g_gfnSdkModule, "gfnSetupTitle");

    if (fnSetupTitle == NULL)
    {
        return gfnAPINotFound;
    }

    return fnSetupTitle(platformAppId);
}

GfnRuntimeError GfnTitleExited(const char* platformId, const char* platformAppId)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnTitleExitedFn fnTitleExited = (gfnTitleExitedFn)GetProcAddress(g_gfnSdkModule, "gfnTitleExited");

    if (fnTitleExited == NULL)
    {
        return gfnAPINotFound;
    }

    return fnTitleExited(platformId, platformAppId);
}

GfnRuntimeError GfnRegisterExitCallback(ExitCallbackSig exitCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterExitCallbackFn fnRegisterExitCallback = (gfnRegisterExitCallbackFn)GetProcAddress(g_gfnSdkModule, "gfnRegisterExitCallback");

    if (fnRegisterExitCallback == NULL)
    {
        return gfnAPINotFound;
    }

    return fnRegisterExitCallback(exitCallback, userContext);
}

GfnRuntimeError GfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterPauseCallbackFn fnRegisterPauseCallback = (gfnRegisterPauseCallbackFn)GetProcAddress(g_gfnSdkModule, "gfnRegisterPauseCallback");

    if (fnRegisterPauseCallback == NULL)
    {
        return gfnAPINotFound;
    }

    return fnRegisterPauseCallback(pauseCallback, userContext);
}

GfnRuntimeError GfnRegisterInstallCallback(InstallCallbackSig installCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterInstallCallbackFn fnRegisterInstallCallback = (gfnRegisterInstallCallbackFn)GetProcAddress(g_gfnSdkModule, "gfnRegisterInstallCallback");

    if (fnRegisterInstallCallback == NULL)
    {
        return gfnAPINotFound;
    }

    return fnRegisterInstallCallback(installCallback, userContext);
}

GfnRuntimeError GfnRegisterSaveCallback(SaveCallbackSig saveCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterSaveCallbackFn fnRegisterSaveCallback = (gfnRegisterSaveCallbackFn)GetProcAddress(g_gfnSdkModule, "gfnRegisterSaveCallback");

    if (fnRegisterSaveCallback == NULL)
    {
        return gfnAPINotFound;
    }

    return fnRegisterSaveCallback(saveCallback, userContext);
}
#endif //end Win32
