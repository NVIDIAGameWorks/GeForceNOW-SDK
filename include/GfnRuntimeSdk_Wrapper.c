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


#include "GfnRuntimeSdk_Wrapper.h"
#ifdef _WIN32
#include "GfnSdk_SecureLoadLibrary.h"
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#   include <ShlObj.h>
#   include <shlwapi.h>
#   ifdef _WIN64
#       define GFN_DLL L"GFN_V2.dll"
#   else
#       define GFN_DLL L"GFN32_V2.dll"
#   endif
#   define GFN_CLIENT_SHARED_LIBRARY L"GfnRuntimeSdk.dll"
#   define GFN_DLL_SUBPATH L"\\NVIDIA Corporation\\GeForceNOW\\SDK\\" GFN_DLL
#   define PLATFORM_MAX_PATH MAX_PATH
    HMODULE g_gfnSdkModule;
    CHAR_TYPE g_cloudLibraryPath[PLATFORM_MAX_PATH];
#elif __linux__
#   include <time.h>        // time functions for log prints
#   include <limits.h>      // PATH_MAX
#   include <sys/types.h>   // stat
#   include <sys/stat.h>    // stat
#   include <dlfcn.h>       // dlopen
#   include <libgen.h>      // dirname
#   include <unistd.h>      // readlink
#   define GFN_SHARED_OBJECT "GfnSdk.so"
#   define GFN_CLIENT_SHARED_LIBRARY "GfnRuntimeSdk.so"
#   define GFN_SHARED_OBJECT_PATH "/opt/nvidia/GfnSdk/" GFN_SHARED_OBJECT
#   define PLATFORM_MAX_PATH PATH_MAX

    void* g_gfnSdkModule = NULL;
    CHAR_TYPE g_cloudLibraryPath[] = GFN_SHARED_OBJECT_PATH;
#else
#   error "Unsupported platform"
#endif

#   define GFN_SDK_INIT_LOGGING() gfnInitLogging();
#   define GFN_SDK_DEINIT_LOGGING() gfnDeinitLogging();
#   define GFN_SDK_LOG(fmt, ...) gfnLog(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#   define kGfnLogBufLen 1024
    typedef struct gfnLogData
    {
        char buffer[kGfnLogBufLen];
#if _WIN32
        SYSTEMTIME timeBuffer;
#elif __linux__
        struct tm timeBuffer;
#endif
    } gfnLogData;
    static gfnLogData s_logData;
    static FILE* s_logfile = NULL;
    static void gfnLog(char const* func, int line, char const* format, ...);
    static void gfnInitLogging();
    static void gfnDeinitLogging();
bool g_LoggingInitialized = false;

// Function declarations
GfnRuntimeError GfnInitializeSdkFromPathDefault(GfnDisplayLanguage language, const CHAR_TYPE* sdkLibraryPath);

// Generic callback function pointer used to wrap the typed callbacks
typedef void (GFN_CALLBACK* _cb)(int, void* pOptionalData, void* pContext);

// Library export function definitions
typedef GfnRuntimeError(*gfnInitializeRuntimeSdkFn)(GfnDisplayLanguage language);
typedef void(*gfnShutdownRuntimeSdkFn)(void);
typedef bool (*gfnIsInitializedFn)();
typedef GfnRuntimeError(*gfnCloudInitializeRuntimeSdkFn)(float libVersion);   // Old Initialization method. Deprecate when all libraries have updated to 1.7.1 or greater.
typedef GfnRuntimeError(*gfnCloudInitializeRuntimeSdkV3Fn)(char* strLibVersion);
typedef void(*gfnCloudShutdownRuntimeSdkFn)(void);
typedef bool(*gfnIsRunningInCloudFn)(void);
typedef GfnRuntimeError(*gfnIsRunningInCloudSecureFn)(GfnIsRunningInCloudAssurance* assurance);
typedef GfnRuntimeError(*gfnCloudCheckFn)(const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, bool* isCloudEnvironment);
typedef GfnRuntimeError(*gfnGetClientIpFn)(const char** cientIp);
typedef GfnRuntimeError(*gfnGetClientLanguageCodeFn)(const char** clientLanguageCode);
typedef GfnRuntimeError(*gfnGetClientCountryCodeFn)(char* clientCountryCode, unsigned int length);

typedef GfnRuntimeError(*gfnGetClientInfoFn)(GfnClientInfo* clientInfo);
typedef GfnRuntimeError(*gfnRegisterClientInfoCallbackFn)(ClientInfoCallbackSig clientInfoCallback, void* pUserContext);
typedef GfnRuntimeError(*gfnRegisterNetworkStatusCallbackFn)(NetworkStatusCallbackSig networkStatusCallback, unsigned int updateRateMs, void* pUserContext);
typedef GfnRuntimeError(*gfnRegisterMessageCallbackFn)(MessageCallbackSig messageCallback, void* pUserContext);

typedef GfnRuntimeError(*gfnGetSessionInfoFn)(GfnSessionInfo* sessionInfo);

typedef GfnRuntimeError(*gfnGetPartnerDataFn)(const char** partnerData);
typedef GfnRuntimeError(*gfnGetPartnerSecureDataFn)(const char** partnerSecureData);

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
typedef GfnRuntimeError(*gfnRegisterCallbackFn)(_cb callback, void* userContext);
typedef GfnRuntimeError(*gfnRegisteCallbackFnWithUIntParam)(_cb callback, unsigned int param, void* userContext);
typedef GfnRuntimeError(*gfnAppReadyFn)(bool success, const char* status);
typedef GfnRuntimeError (*gfnSetActionZoneFn)(GfnActionType type, unsigned int id, GfnRect* zone);
typedef GfnRuntimeError(*gfnSendMessageFn)(const char* pchMessage, unsigned int length);

typedef struct GfnSdkCloudLibrary_t
{
    void* handle;
    gfnCloudInitializeRuntimeSdkFn InitializeRuntimeSdk;            // Old Initialization method. Deprecate when all libraries have updated to 1.7.1 or greater.
    gfnCloudInitializeRuntimeSdkV3Fn InitializeRuntimeSdkV3;
    gfnCloudShutdownRuntimeSdkFn ShutdownRuntimeSdk;
    gfnIsInitializedFn IsInitialized;
    gfnIsRunningInCloudFn IsRunningInCloud;
    gfnIsRunningInCloudSecureFn IsRunningInCloudSecure;
    gfnCloudCheckFn CloudCheck;
    gfnRegisterCallbackFn RegisterExitCallback;
    gfnRegisterCallbackFn RegisterSaveCallback;
    gfnRegisterCallbackFn RegisterSessionInitCallback;
    gfnRegisterCallbackFn RegisterPauseCallback;
    gfnRegisterCallbackFn RegisterInstallCallback;
    gfnIsTitleAvailableFn IsTitleAvailable;
    gfnGetTitlesAvailableFn GetTitlesAvailable;
    gfnSetupTitleFn SetupTitle;
    gfnTitleExitedFn TitleExited;
    gfnGetClientIpFn GetClientIp;
    gfnGetClientLanguageCodeFn GetClientLanguageCode;
    gfnGetClientCountryCodeFn GetClientCountryCode;
    gfnGetPartnerDataFn GetPartnerData;
    gfnGetPartnerSecureDataFn GetPartnerSecureData;
    gfnFreeFn Free;
    gfnAppReadyFn AppReady;
    gfnSetActionZoneFn SetActionZone;
    gfnSendMessageFn SendMessage;

    gfnGetClientInfoFn GetClientInfo;
    gfnRegisterCallbackFn RegisterClientInfoCallback;
    gfnRegisteCallbackFnWithUIntParam RegisterNetworkStatusCallback;
    gfnRegisterCallbackFn RegisterMessageCallback;

    gfnGetSessionInfoFn GetSessionInfo;
} GfnSdkCloudLibrary;
GfnSdkCloudLibrary* g_pCloudLibrary = NULL;
GfnRuntimeError g_cloudLibraryStatus = gfnAPINotInit;

inline bool GfnUtf8ToWide(const char* in, wchar_t* out, int outSize)
{
#ifdef _WIN32
    int result = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
    if (result <= 0 || outSize < result)
    {
        return false;
    }

    result = MultiByteToWideChar(CP_UTF8, 0, in, -1, out, outSize);
    if (result <= 0)
    {
        return false;
    }
    return true;
#elif __linux__
    return false;
#endif
}

inline bool GfnWideToUtf8(const wchar_t* in, char* out, int outSize)
{
#ifdef _WIN32
    int length = WideCharToMultiByte(CP_UTF8, 0, in, -1, NULL, 0, NULL, NULL);
    if (length <= 0 || outSize < length)
    {
        return false;
    }
    length = WideCharToMultiByte(CP_UTF8, 0, in, -1, out, outSize, NULL, NULL);
    if (length <= 0)
    {
        return false;
    }
    return true;
#elif __linux__
    return false;
#endif
}

static void gfnFreeLibrary(void* library)
{
#ifdef _WIN32
    FreeLibrary(library);
#elif __linux__
    dlclose(library);
#endif
}

static void gfnFreeCloudLibrary(GfnSdkCloudLibrary* pCloudLibrary)
{
    if (pCloudLibrary != NULL)
    {
        if (pCloudLibrary->handle)
        {
            gfnFreeLibrary(pCloudLibrary->handle);
        }
        free(pCloudLibrary);
    }
}

static GfnRuntimeError gfnTranslateCloudStatus(int status)
{
    return (GfnRuntimeError)status;
}

GfnRuntimeError gfnShutDownCloudSdk(void)
{
    if (g_pCloudLibrary != NULL)
    {
        if (g_pCloudLibrary->ShutdownRuntimeSdk != NULL)
        {
            g_pCloudLibrary->ShutdownRuntimeSdk();
        }

        gfnFreeCloudLibrary(g_pCloudLibrary);
        g_pCloudLibrary = NULL;
        g_cloudLibraryStatus = gfnAPINotInit;
    }

    return gfnSuccess;
}

bool gfnPathExists(const CHAR_TYPE* path)
{
#ifdef _WIN32
    return (PathFileExistsW(path) != FALSE);
#elif __linux__
    struct stat buffer;
    return (stat(path, &buffer) == 0);
#endif
}

const CHAR_TYPE* gfnGetFilenameFromPath(const CHAR_TYPE* path)
{
#ifdef _WIN32
    CHAR_TYPE *lastBackSepPos = wcsrchr(path, L'/');
    CHAR_TYPE *lastForeSepPos = wcsrchr(path, L'\\');
#elif __linux__
    CHAR_TYPE *lastBackSepPos = strrchr(path, '/');
    CHAR_TYPE *lastForeSepPos = strrchr(path, '\\');
#endif

    CHAR_TYPE* lastSepPos = (lastBackSepPos != NULL && lastBackSepPos > lastForeSepPos) ? lastBackSepPos : lastForeSepPos;

    if (lastSepPos == NULL)
    {
        return path;
    }
    else
    {
        return lastSepPos + 1;
    }
}

// This is case sensitive for POSIX, insensitive for WIN32
bool gfnPathEqual(const CHAR_TYPE* path1, const CHAR_TYPE* path2)
{
#ifdef _WIN32
    return _wcsicmp(path1, path2) == 0;
#elif __linux__
    return strcmp(path1, path2) == 0;
#endif
}

void* gfnLoadLibrary(const CHAR_TYPE* path)
{
    // For security reasons, it is preferred to check the digital signature before loading the DLL.
    // Such code is not provided here to reduce code complexity and library size, and in favor of
    // any internal libraries built for this purpose.
#ifdef _WIN32
#   ifdef _DEBUG
        return LoadLibraryW(path);
#   else
        return gfnSecureLoadCloudLibraryW(path, 0);
#   endif
#elif __linux__
    return dlopen(path, RTLD_LAZY);
#endif
}

void* gfnGetSymbol(void* library, char* name)
{
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)library, name);
#elif __linux__
    return dlsym(library, name);
#endif
}

GfnRuntimeError gfnGetDefaultClientLibraryPath(CHAR_TYPE* path)
{
#ifdef _WIN32
    wchar_t* filename = (wchar_t*)malloc(sizeof(wchar_t) * PLATFORM_MAX_PATH);
    DWORD pathSize = GetModuleFileNameW(NULL, filename, PLATFORM_MAX_PATH);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        free(filename);
        filename = (wchar_t*)malloc(sizeof(wchar_t) * (pathSize + 1)); //Room for \0
        if (!filename)
        {
            return gfnInternalError;
        }
        pathSize = GetModuleFileNameW(NULL, filename, pathSize + 1);
    }

    wchar_t* lastSepPos = wcsrchr(filename, L'\\');
    size_t pathLength = lastSepPos - filename + 1;
    size_t dllPathLength = pathLength + 18; // "GfnRuntimeSdk.dll\0"

    if (!path)
    {
        free(filename);
        return gfnInternalError;
    }
    wcsncpy_s(path, dllPathLength, filename, _TRUNCATE);

    path[pathLength] = L'\0';
    free(filename);
    wcsncat_s(path, dllPathLength, L"GfnRuntimeSdk.dll", 17);
#elif __linux__
    char buffer[PLATFORM_MAX_PATH];
    ssize_t len;

    // Get the absolute path of the executable
    len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

    // Check for errors
    if (len == -1) {
        return gfnInternalError;
    }

    // Null-terminate the string
    buffer[len] = '\0';

    // Get the directory path, excluding the executable name
    char* directory = dirname(buffer);
    strncpy(path, directory, PLATFORM_MAX_PATH);

    // Append "GfnRuntimeSdk.so" to the directory path
    if (strlen(path) + strlen("/" GFN_CLIENT_SHARED_LIBRARY) + 1 > PLATFORM_MAX_PATH) {
        GFN_SDK_LOG("ERROR: Could not get default client library path name: Path too long");
        return gfnInternalError;
    }

    strcat(path, "/GfnRuntimeSdk.so");
#endif
    return gfnSuccess;
}

// Places the null terminator where the path ends
GfnRuntimeError gfnStripFilename(char* path)
{
    char *lastBackSepPos = strrchr(path, '/');
    char *lastForeSepPos = strrchr(path, '\\');
    char* lastSepPos = (lastBackSepPos != NULL && lastBackSepPos > lastForeSepPos) ? lastBackSepPos : lastForeSepPos;
    if (lastSepPos)
    {
        *lastSepPos = '\0';
    }
    else
    {
        return gfnInvalidParameter;
    }
    return gfnSuccess;
}

GfnRuntimeError gfnLoadCloudLibrary(GfnSdkCloudLibrary** ppCloudLibrary)
{
    // If we've already attempted to load this, return the previous results and library
    if (g_cloudLibraryStatus != gfnAPINotInit)
    {
        *ppCloudLibrary = g_pCloudLibrary;
        return g_cloudLibraryStatus;
    }

    *ppCloudLibrary = NULL;

#ifdef _WIN32
    if (SHGetSpecialFolderPathW(NULL, g_cloudLibraryPath, CSIDL_PROGRAM_FILES, false) == TRUE)
    {
        if (wcscat_s(g_cloudLibraryPath, PLATFORM_MAX_PATH, GFN_DLL_SUBPATH) != 0)
        {
            GFN_SDK_LOG("FAIL: Unable to concatenate path to Runtime SDK binaries");
            return gfnInitFailure;
        }
    }
    else
    {
        GFN_SDK_LOG("FAIL: Unable to get path to Runtime SDK binaries");
        return gfnInitFailure;
    }
#endif // _WIN32

    if (!gfnPathExists(g_cloudLibraryPath))
    {
        GFN_SDK_LOG("SUCCESS: Cloud library does not exist, this is running on the user client");
        return gfnCloudLibraryNotFound;
    }

    void* library = gfnLoadLibrary(g_cloudLibraryPath);
    if (!library)
    {
#ifdef _WIN32
        DWORD lastError = GetLastError();
        if (lastError == CRYPT_E_NO_MATCH)
        {
            GFN_SDK_LOG("ERROR: GFN library failed to load due to invalid signature");
            return gfnBinarySignatureInvalid;
        }
        else
        {
            GFN_SDK_LOG("ERROR: GFN library is present but unable to be loaded! LastError=0x%08X", lastError);
            return gfnInitFailure;
        }
#elif __linux__
        GFN_SDK_LOG("GFN library is present but unable to be loaded! dlerror=%s", dlerror());
        return gfnInitFailure;
#endif
    }

    GfnSdkCloudLibrary* pCloudLibrary = (GfnSdkCloudLibrary*)malloc(sizeof(GfnSdkCloudLibrary));
    if (pCloudLibrary == NULL)
    {
        GFN_SDK_LOG("ERROR: Unable to allocate memory to hold GFN function pointers");
        gfnFreeLibrary(library);
        return gfnUnableToAllocateMemory;
    }

    pCloudLibrary->handle = library;
    // Old Initialization method. Deprecate when all libraries have updated to 1.7.1 or greater.
    pCloudLibrary->InitializeRuntimeSdk = (gfnCloudInitializeRuntimeSdkFn)gfnGetSymbol(pCloudLibrary->handle, "gfnInitializeRuntimeSdk2");
    pCloudLibrary->InitializeRuntimeSdkV3 = (gfnCloudInitializeRuntimeSdkV3Fn)gfnGetSymbol(pCloudLibrary->handle, "gfnInitializeRuntimeSdk3");
    pCloudLibrary->ShutdownRuntimeSdk = (gfnCloudShutdownRuntimeSdkFn)gfnGetSymbol(pCloudLibrary->handle, "gfnShutdownRuntimeSdk2");
    pCloudLibrary->IsInitialized = (gfnIsInitializedFn)gfnGetSymbol(pCloudLibrary->handle, "gfnIsInitialized");
    pCloudLibrary->IsRunningInCloud = (gfnIsRunningInCloudFn)gfnGetSymbol(pCloudLibrary->handle, "gfnIsRunningInCloud");
    pCloudLibrary->IsRunningInCloudSecure = (gfnIsRunningInCloudSecureFn)gfnGetSymbol(pCloudLibrary->handle, "gfnIsRunningInCloudSecure");
    pCloudLibrary->CloudCheck = (gfnCloudCheckFn)gfnGetSymbol(pCloudLibrary->handle, "gfnCloudCheck");
    pCloudLibrary->IsTitleAvailable = (gfnIsTitleAvailableFn)gfnGetSymbol(pCloudLibrary->handle, "gfnIsTitleAvailable");
    pCloudLibrary->GetTitlesAvailable = (gfnGetTitlesAvailableFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetTitlesAvailable");
    pCloudLibrary->SetupTitle = (gfnSetupTitleFn)gfnGetSymbol(pCloudLibrary->handle, "gfnSetupTitle");
    pCloudLibrary->TitleExited = (gfnTitleExitedFn)gfnGetSymbol(pCloudLibrary->handle, "gfnTitleExited");
    pCloudLibrary->GetClientIp = (gfnGetClientIpFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetClientIp");
    pCloudLibrary->GetClientLanguageCode = (gfnGetClientLanguageCodeFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetClientLanguageCode");
    pCloudLibrary->GetClientCountryCode = (gfnGetClientCountryCodeFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetClientCountryCode");
    pCloudLibrary->GetPartnerData = (gfnGetPartnerDataFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetPartnerData");
    pCloudLibrary->GetPartnerSecureData = (gfnGetPartnerSecureDataFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetPartnerSecureData");
    pCloudLibrary->Free = (gfnFreeFn)gfnGetSymbol(pCloudLibrary->handle, "gfnFree");
    pCloudLibrary->AppReady = (gfnAppReadyFn)gfnGetSymbol(pCloudLibrary->handle, "gfnAppReady");
    pCloudLibrary->SetActionZone = (gfnSetActionZoneFn)gfnGetSymbol(pCloudLibrary->handle, "gfnSetActionZone");
    pCloudLibrary->SendMessage = (gfnSendMessageFn)gfnGetSymbol(pCloudLibrary->handle, "gfnSendCustomMessageToClient");
    pCloudLibrary->RegisterExitCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterExitCallback");
    pCloudLibrary->RegisterPauseCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterPauseCallback");
    pCloudLibrary->RegisterInstallCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterInstallCallback");
    pCloudLibrary->RegisterSaveCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterSaveCallback");
    pCloudLibrary->RegisterSessionInitCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterSessionInitCallback");

    pCloudLibrary->GetClientInfo = (gfnGetClientInfoFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetClientInfo");
    pCloudLibrary->RegisterClientInfoCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterClientInfoCallback");
    pCloudLibrary->RegisterNetworkStatusCallback = (gfnRegisteCallbackFnWithUIntParam)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterNetworkStatusCallback");
    pCloudLibrary->RegisterMessageCallback = (gfnRegisterCallbackFn)gfnGetSymbol(pCloudLibrary->handle, "gfnRegisterCustomMessageCallback");
    pCloudLibrary->GetSessionInfo = (gfnGetSessionInfoFn)gfnGetSymbol(pCloudLibrary->handle, "gfnGetSessionInfo");

    GFN_SDK_LOG("Successfully loaded cloud libary");

    if (pCloudLibrary->InitializeRuntimeSdk == NULL && pCloudLibrary->InitializeRuntimeSdkV3 == NULL)
    {
        GFN_SDK_LOG("Unable to find initialize function pointer");
        gfnFreeCloudLibrary(pCloudLibrary);
        return gfnAPINotFound;
    }

    *ppCloudLibrary = pCloudLibrary;

    return gfnSuccess;
}

GfnRuntimeError gfnInitializeCloudSdk(void)
{
    // Already initialized, no need to re-initialize
    if (g_pCloudLibrary != NULL)
    {
        return g_cloudLibraryStatus;
    }

    g_cloudLibraryStatus = gfnLoadCloudLibrary(&g_pCloudLibrary);
    if (g_cloudLibraryStatus != gfnSuccess)
    {
        return g_cloudLibraryStatus;
    }

    if (g_pCloudLibrary->InitializeRuntimeSdkV3)
    {
        g_cloudLibraryStatus = g_pCloudLibrary->InitializeRuntimeSdkV3(NVGFNSDK_VERSION_STR);
    }
    // Old Initialization method. Deprecate when all libraries have updated to 1.7.1 or greater.
    else if (g_pCloudLibrary->InitializeRuntimeSdk)
    {
        g_cloudLibraryStatus = g_pCloudLibrary->InitializeRuntimeSdk((float)(NVGFNSDK_VERSION_SHORT));
    }
    if (GFNSDK_FAILED(g_cloudLibraryStatus))
    {
        GFN_SDK_LOG("Call to cloud InitializeRuntimeSdk failed: %d", g_cloudLibraryStatus);
        // If init fails, we shouldn't force the host application to hold a loaded reference to the cloud DLL.
        // Instead we will unload to make sure SDK is in a clean state in case the application tried to call
        // the Initialize API again.
        gfnFreeCloudLibrary(g_pCloudLibrary);
        g_pCloudLibrary = NULL;
        g_cloudLibraryStatus = gfnAPINotInit;
    }
    return g_cloudLibraryStatus;
}

typedef struct _gfnUserContextCallbackWrapper
{
    void* fnCallback;
    void* pOrigUserContext;
} _gfnUserContextCallbackWrapper;

enum IsCloud{IsCloud_Unknown,IsCloud_Yes,IsCloud_No};
static enum IsCloud g_isCloud = IsCloud_Unknown;

#define CHECK_NULL_PARAM(param)         \
    if (!param)                         \
    {                                   \
        return gfnInvalidParameter;     \
    }
#define CHECK_CLOUD_ENVIRONMENT_IMPL(bUseCache)                                             \
    if(g_isCloud == IsCloud_Unknown || !bUseCache)                                          \
    {                                                                                       \
        if (!g_pCloudLibrary && !g_gfnSdkModule)                                            \
        {                                                                                   \
            return gfnAPINotInit;                                                           \
        }                                                                                   \
        if (!g_pCloudLibrary || !g_pCloudLibrary->IsRunningInCloud)                         \
        {                                                                                   \
            GFN_SDK_LOG("Cannot call cloud function: Wrong environment");                   \
            return gfnCallWrongEnvironment;                                                 \
        }                                                                                   \
        g_isCloud = ((bool)g_pCloudLibrary->IsRunningInCloud()) ? IsCloud_Yes : IsCloud_No; \
    }                                                                                       \
    if (g_isCloud == IsCloud_No)                                                            \
    {                                                                                       \
        GFN_SDK_LOG("Cannot call cloud function: Wrong environment");                       \
        return gfnCallWrongEnvironment;                                                     \
    }
#define CHECK_CLOUD_ENVIRONMENT() CHECK_CLOUD_ENVIRONMENT_IMPL(true)
#define CHECK_CLOUD_API_AVAILABLE(Fn)                                       \
    if (g_pCloudLibrary->Fn == NULL)                                        \
    {                                                                       \
        GFN_SDK_LOG("Cannot call cloud function %s: API not found", #Fn);   \
        return gfnAPINotFound;                                              \
    }
#define DELEGATE_TO_CLOUD_LIBRARY(Fn, ...)                              \
    CHECK_CLOUD_API_AVAILABLE(Fn);                                      \
    return gfnTranslateCloudStatus(g_pCloudLibrary->Fn(__VA_ARGS__));

GfnRuntimeError GfnInitializeSdk(GfnDisplayLanguage language)
{
    // If "client" SDK is already initialized, then we're good to go.
    // "server" SDK may or may not be initialized depending on mode.
    GfnRuntimeError clientStatus = gfnSuccess;

    if (!g_LoggingInitialized)
    {
        GFN_SDK_INIT_LOGGING();
        g_LoggingInitialized = true;
    }


    if (g_gfnSdkModule != NULL)
    {
        GFN_SDK_LOG("Client library already initialized, no need to initialize again");
    }
    else
    {
        CHAR_TYPE* filename = (CHAR_TYPE*)malloc(sizeof(CHAR_TYPE) * PLATFORM_MAX_PATH);
        if (!filename)
        {
            return gfnInternalError;
        }

        GfnRuntimeError err = gfnGetDefaultClientLibraryPath(filename);
        if (GFNSDK_FAILED(err))
        {
            free(filename);
            return err;
        }

        clientStatus = GfnInitializeSdkFromPathDefault(language, filename);
        free(filename);
    }

    if (GFNSDK_FAILED(clientStatus))
    {
        GFN_SDK_LOG("Initialization failed: %d", clientStatus);
        GfnShutdownSdk();
    }

    return clientStatus;
}

// On WIN32, this accepts a wide char string.
// On all other platforms, this accepts a UTF-8 string.
GfnRuntimeError GfnInitializeSdkFromPathDefault(GfnDisplayLanguage language, const CHAR_TYPE* sdkLibraryPath)
{
    // If "client" library is already initialized, then we're good to go.
    if (g_gfnSdkModule != NULL)
    {
        GFN_SDK_LOG("Client library already initialized, no need to initialize again");
        return gfnSuccess;
    }

    GfnRuntimeError clientStatus = gfnSuccess;
    if (!g_LoggingInitialized)
    {
        GFN_SDK_INIT_LOGGING();
        g_LoggingInitialized = true;
    }

    const CHAR_TYPE* filename = gfnGetFilenameFromPath(sdkLibraryPath);
    if (!filename || !gfnPathEqual(filename, GFN_CLIENT_SHARED_LIBRARY))
    {
        GFN_SDK_LOG("Invalid SDK library name");
        return gfnInvalidParameter;
    }

    if (!gfnPathExists(sdkLibraryPath))
    {
        clientStatus = gfnClientLibraryNotFound;
    }
    else
    {

        GFN_SDK_LOG("Initializing the GfnSdk");
        // For security reasons, it is preferred to check the digital signature before loading the DLL.
        // Such code is not provided here to reduce code complexity and library size, and in favor of
        // any internal libraries built for this purpose.
        g_gfnSdkModule = gfnLoadLibrary(sdkLibraryPath);
        if (g_gfnSdkModule == NULL)
        {
            clientStatus = gfnClientLibraryNotFound;
#ifdef _WIN32
            DWORD lastError = GetLastError();
            if (lastError == CRYPT_E_NO_MATCH)
            {
                GFN_SDK_LOG("ERROR: GFN library failed to load due to invalid signature");
                clientStatus = gfnBinarySignatureInvalid;
            }
#elif __linux__
            GFN_SDK_LOG("GFN client library is present but unable to be loaded! dlerror=%s", dlerror());
#endif
        }
        else
        {
            gfnInitializeRuntimeSdkFn fnGfnInitializeRuntimeSdk = (gfnInitializeRuntimeSdkFn)gfnGetSymbol(g_gfnSdkModule, "gfnInitializeRuntimeSdk");
            if (fnGfnInitializeRuntimeSdk == NULL)
            {
                clientStatus = gfnAPINotFound;
            }
            else
            {
                clientStatus = (fnGfnInitializeRuntimeSdk)(language);
            }
        }
    }
    // The gfnClientLibraryNotFound error means client library was not present.
    // This is allowed in the GFN cloud environment for robustness reasons as all API
    // calls are deferred to the cloud library, although this is not a recommended use case.
    // Any other error, including presence of a client library that couldn't be validated, is fatal.
    if (GFNSDK_FAILED(clientStatus) && clientStatus != gfnClientLibraryNotFound)
    {
        GFN_SDK_LOG("Client SDK library init failed: %d", clientStatus);
        GfnShutdownSdk();
        return clientStatus;
    }

    // With the client library loaded attempt to load the cloud Sdk library if available (inside GFN only) in order to use it
    // directly for cloud API calls.
    GfnRuntimeError cloudStatus = gfnInitializeCloudSdk();
    // gfnCloudLibraryNotFound is allowed, indicating that this is not running in a cloud environment.
    // All other errors are fatal.
    if (GFNSDK_FAILED(cloudStatus) && (cloudStatus != gfnCloudLibraryNotFound))
    {
        GFN_SDK_LOG("Cloud library init failed: %d", cloudStatus);
        GfnShutdownSdk();
        return cloudStatus;
    }

    // If we could find either SDK library, then this is a fatal condition.
    if (clientStatus == gfnClientLibraryNotFound && cloudStatus == gfnCloudLibraryNotFound)
    {
        GFN_SDK_LOG("Failed to find any valid SDK libraries");
        return clientStatus;
    }

    GFN_SDK_LOG("Initialization successful");

    if (GFNSDK_SUCCEEDED(cloudStatus) && clientStatus == gfnClientLibraryNotFound)
    {
        return gfnInitSuccessCloudOnly;
    }
    if (GFNSDK_SUCCEEDED(clientStatus) && cloudStatus == gfnCloudLibraryNotFound)
    {
        return gfnInitSuccessClientOnly;
    }

    return gfnSuccess;
}

GfnRuntimeError GfnInitializeSdkFromPathA(GfnDisplayLanguage language, const char* utf8SdkLibraryPath)
{
    if (utf8SdkLibraryPath == NULL)
    {
        GFN_SDK_LOG("Invalid SDK library path");
        return gfnInvalidParameter;
    }

#ifdef _WIN32
    // On windows, first convert to Unicode-16, then call the implementation.
    size_t libPathSize = strlen(utf8SdkLibraryPath) + 1;
    wchar_t* wSdkLibraryPath = (wchar_t*)malloc(libPathSize * sizeof(wchar_t));
    if (!wSdkLibraryPath)
    {
        GFN_SDK_LOG("Failed to allocate for SDK library path");
        return gfnUnableToAllocateMemory;
    }
    int outSize = (int)libPathSize * sizeof(wchar_t);
    if (!GfnUtf8ToWide(utf8SdkLibraryPath, wSdkLibraryPath, outSize))
    {
        GFN_SDK_LOG("Failed to convert SDK library path");
        free(wSdkLibraryPath);
        return gfnInternalError;
    }

    GfnError status = GfnInitializeSdkFromPathDefault(language, wSdkLibraryPath);
    free(wSdkLibraryPath);
    return status;
#elif __linux__
    // On linux, this is the default encoding. Directly call the implementation.
    return GfnInitializeSdkFromPathDefault(language, utf8SdkLibraryPath);
#endif
}

GfnRuntimeError GfnInitializeSdkFromPathW(GfnDisplayLanguage language, const wchar_t* wSdkLibraryPath)
{
    if (wSdkLibraryPath == NULL)
    {
        GFN_SDK_LOG("Invalid SDK library path");
        return gfnInvalidParameter;
    }

#ifdef _WIN32
    // On windows, this is the default encoding. Directly call the implementation.
    return GfnInitializeSdkFromPathDefault(language, wSdkLibraryPath);
#elif __linux__
    GFN_SDK_LOG("GfnInitializeSdkFromPathW is unsupported on linux");
    return gfnInvalidParameter;
#endif
}

GfnRuntimeError GfnShutdownSdk(void)
{
    gfnShutDownCloudSdk();

    if (g_gfnSdkModule == NULL)
    {
        // Not initialized, no need to shutdown
        return gfnSuccess;
    }

    gfnShutdownRuntimeSdkFn fnGfnShutdownRuntimeSdk = (gfnShutdownRuntimeSdkFn)gfnGetSymbol(g_gfnSdkModule, "gfnShutdownRuntimeSdk");
    if (fnGfnShutdownRuntimeSdk == NULL)
    {
        return gfnAPINotFound;
    }

    fnGfnShutdownRuntimeSdk();

    gfnFreeLibrary(g_gfnSdkModule);
    g_gfnSdkModule = NULL;

    GFN_SDK_DEINIT_LOGGING();
    return gfnSuccess;
}

GfnRuntimeError GfnIsRunningInCloud(bool* runningInCloud)
{
    CHECK_NULL_PARAM(runningInCloud);
    *runningInCloud = false;

    if (g_pCloudLibrary == NULL && g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    if (g_pCloudLibrary == NULL)
    {
        GFN_SDK_LOG("No cloud library present, call succeeds");
        return gfnSuccess;
    }

    if (g_pCloudLibrary->IsRunningInCloud == NULL)
    {
        GFN_SDK_LOG("API Not Found");
        return gfnAPINotFound;
    }

    *runningInCloud = (bool)g_pCloudLibrary->IsRunningInCloud();

    GFN_SDK_LOG("Success: %d", *runningInCloud);
    return gfnSuccess;
}

GfnRuntimeError GfnIsRunningInCloudSecure(GfnIsRunningInCloudAssurance* assurance)
{
    CHECK_NULL_PARAM(assurance);
    *assurance = gfnNotCloud;

    if (g_pCloudLibrary == NULL && g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

#ifdef _WIN32
    if (wcslen(g_cloudLibraryPath) == 0)
#elif __linux__
    if (strlen(g_cloudLibraryPath) == 0)
#endif
    {
        GFN_SDK_LOG("Cloud library path not defined, which denotes an API call without Initialize succeeding, treating as not in cloud");
        return gfnSuccess;
    }

#ifdef _WIN32
    if (gfnCheckLibraryGfnSignatureW(g_cloudLibraryPath) == FALSE)
    {
        GFN_SDK_LOG("Cloud library path does not have valid GFN signing, treating as not in cloud");
        return gfnSuccess;
    }
#endif

    if (g_pCloudLibrary == NULL)
    {
        GFN_SDK_LOG("No cloud library present, call succeeds");
        return gfnSuccess;
    }

    if (g_pCloudLibrary->IsRunningInCloudSecure == NULL)
    {
        GFN_SDK_LOG("API Not Found");
        return gfnAPINotFound;
    }

    GfnRuntimeError status = gfnTranslateCloudStatus(g_pCloudLibrary->IsRunningInCloudSecure(assurance));
    GFN_SDK_LOG("status=%d assurance=%d", status, *assurance);

    return status;
}

GfnRuntimeError GfnCloudCheck(const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, bool* isCloudEnvironment)
{
    *isCloudEnvironment = false;

    if (g_pCloudLibrary == NULL && g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

#ifdef _WIN32
    if (wcslen(g_cloudLibraryPath) == 0)
#elif __linux__
    if (strlen(g_cloudLibraryPath) == 0)
#endif
    {
        GFN_SDK_LOG("Cloud library path not defined, which denotes an API call without Initialize succeeding, treating as not in cloud");
        return gfnSuccess;
    }

    if (g_pCloudLibrary == NULL)
    {
        GFN_SDK_LOG("No cloud library present, call succeeds");
        return gfnSuccess;
    }

    if (g_pCloudLibrary->CloudCheck == NULL)
    {
        GFN_SDK_LOG("API Not Found");
        return gfnAPINotFound;
    }

    GfnRuntimeError status = gfnTranslateCloudStatus(g_pCloudLibrary->CloudCheck(challenge, response, isCloudEnvironment));
    GFN_SDK_LOG("status=%d isCloudEnvironment=%d", status, *isCloudEnvironment);

    return status;
}

#define TESTME(lib, fn) lib->fn()

GfnRuntimeError GfnFree(const char** data)
{
    CHECK_NULL_PARAM(data);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(Free, data);
}

GfnRuntimeError GfnGetClientIpV4(const char ** clientIp)
{
    CHECK_NULL_PARAM(clientIp);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetClientIp, clientIp);
}

GfnRuntimeError GfnGetClientLanguageCode(const char** languageCode)
{
    CHECK_NULL_PARAM(languageCode);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetClientLanguageCode, languageCode);
}

GfnRuntimeError GfnGetClientCountryCode(char* countryCode, unsigned int length)
{
    CHECK_NULL_PARAM(countryCode);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetClientCountryCode, countryCode, length);
}

GfnRuntimeError GfnGetPartnerData(const char** partnerData)
{
    CHECK_NULL_PARAM(partnerData);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetPartnerData, partnerData);
}

GfnRuntimeError GfnGetPartnerSecureData(const char** partnerSecureData)
{
    CHECK_NULL_PARAM(partnerSecureData);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetPartnerSecureData, partnerSecureData);
}

GfnRuntimeError GfnIsTitleAvailable(const char* platformAppId, bool* isAvailable)
{
    CHECK_NULL_PARAM(isAvailable);
    *isAvailable = false;

    CHECK_NULL_PARAM(platformAppId);
    CHECK_CLOUD_ENVIRONMENT();
    CHECK_CLOUD_API_AVAILABLE(IsTitleAvailable);
    *isAvailable = (bool)g_pCloudLibrary->IsTitleAvailable(platformAppId);

    return gfnSuccess;
}

GfnRuntimeError GfnGetTitlesAvailable(const char** platformAppIds)
{
    CHECK_NULL_PARAM(platformAppIds);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetTitlesAvailable, platformAppIds);
}


GfnRuntimeError GfnGetClientInfo(GfnClientInfo* clientInfo)
{
    GFN_SDK_LOG("Calling GfnGetClientInfo");
    CHECK_NULL_PARAM(clientInfo);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetClientInfo, clientInfo);
}

static void GFN_CALLBACK _gfnClientInfoCallbackWrapper(int status, void* updateData, void* pData)
{
    (void)status;
    GFN_SDK_LOG("ClientInfo update received");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pData);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        GFN_SDK_LOG("Wrapped context was null or had no callback. Ignoring");
        return;
    }
    ClientInfoCallbackSig cb = (ClientInfoCallbackSig)(pWrappedContext->fnCallback);
    if (cb == NULL)
    {
        GFN_SDK_LOG("Callback was NULL, ignoring");
        return;
    }
    cb((GfnClientInfoUpdateData *)updateData, pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterClientInfoCallback(ClientInfoCallbackSig clientInfoCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(clientInfoCallback);
    CHECK_CLOUD_ENVIRONMENT();
    GFN_SDK_LOG("Registering for ClientInfo updates");
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)clientInfoCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterClientInfoCallback, &_gfnClientInfoCallbackWrapper, (void*)(pWrappedContext));
}

GfnRuntimeError GfnGetSessionInfo(GfnSessionInfo* sessionInfo)
{
    GFN_SDK_LOG("Calling GfnGetSessionInfo");
    CHECK_NULL_PARAM(sessionInfo);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(GetSessionInfo, sessionInfo);
}


static void GFN_CALLBACK _gfnNetworkStatusCallbackWrapper(int status, void* updateData, void* pData)
{
    (void)status;
    GFN_SDK_LOG("Network performance update received");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pData);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        GFN_SDK_LOG("Wrapped context was null or had no callback. Ignoring");
        return;
    }
    NetworkStatusCallbackSig cb = (NetworkStatusCallbackSig)(pWrappedContext->fnCallback);
    if (cb == NULL)
    {
        GFN_SDK_LOG("Callback was NULL, ignoring");
        return;
    }
    cb((GfnNetworkStatusUpdateData *)updateData, pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterNetworkStatusCallback(NetworkStatusCallbackSig networkStatusCallback, unsigned int updateRateMs, void* pUserContext)
{
    CHECK_NULL_PARAM(networkStatusCallback);
    CHECK_CLOUD_ENVIRONMENT();
    GFN_SDK_LOG("Registering for NetworkStatus updates");
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)networkStatusCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterNetworkStatusCallback, &_gfnNetworkStatusCallbackWrapper, updateRateMs, (void*)(pWrappedContext));
}

GfnRuntimeError GfnRegisterStreamStatusCallback(StreamStatusCallbackSig streamStatusCallback, void* userContext)
{
    if (g_gfnSdkModule == NULL)
    {
        return gfnAPINotInit;
    }

    gfnRegisterStreamStatusCallbackFn fnRegisterStreamStatusCallback = (gfnRegisterStreamStatusCallbackFn)gfnGetSymbol(g_gfnSdkModule, "gfnRegisterStreamStatusCallback");

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

    gfnStartStreamFn fnGfnStartStream = (gfnStartStreamFn)gfnGetSymbol(g_gfnSdkModule, "gfnStartStream");

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

    gfnStartStreamAsyncFn fnGfnStartStreamAsync = (gfnStartStreamAsyncFn)gfnGetSymbol(g_gfnSdkModule, "gfnStartStreamAsync");

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

    gfnStopStreamFn fnGfnStopStream = (gfnStopStreamFn)gfnGetSymbol(g_gfnSdkModule, "gfnStopStream");

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

    gfnStopStreamAsyncFn fnGfnStopStreamAsync = (gfnStopStreamAsyncFn)gfnGetSymbol(g_gfnSdkModule, "gfnStopStreamAsync");

    if (fnGfnStopStreamAsync == NULL)
    {
        return gfnAPINotFound;
    }

    fnGfnStopStreamAsync(cb, context, timeoutMs);

    return gfnSuccess;
}

GfnRuntimeError GfnSetupTitle(const char* platformAppId)
{
    CHECK_NULL_PARAM(platformAppId);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(SetupTitle, platformAppId);
}

GfnRuntimeError GfnTitleExited(const char* platformId, const char* platformAppId)
{
    CHECK_NULL_PARAM(platformId);
    CHECK_NULL_PARAM(platformAppId);
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(TitleExited, platformId, platformAppId);
}

GfnRuntimeError GfnAppReady(bool success, const char* status)
{
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(AppReady, success, status);
}

GfnRuntimeError GfnSetActionZone(GfnActionType type, unsigned int id, GfnRect* zone)
{
    CHECK_CLOUD_ENVIRONMENT();
    DELEGATE_TO_CLOUD_LIBRARY(SetActionZone, type, id, zone);
}

GfnRuntimeError GfnSendMessage(const char* pchMessage, unsigned int length) {
    if (g_pCloudLibrary != NULL && g_pCloudLibrary->SendMessage != NULL)                                        \
    {                                                                       \
        DELEGATE_TO_CLOUD_LIBRARY(SendMessage, pchMessage, length);              \
    }
    else
    {
        if (g_gfnSdkModule == NULL)
        {
            return gfnAPINotInit;
        }

        gfnSendMessageFn fnSendMessage = (gfnSendMessageFn)gfnGetSymbol(g_gfnSdkModule, "gfnSendMessage");

        if (fnSendMessage == NULL)
        {
            return gfnAPINotFound;
        }

        return fnSendMessage(pchMessage, length);
    }
}

static void GFN_CALLBACK _gfnExitCallbackWrapper(int status, void* pUnused, void* pContext)
{
    (void)status;
    (void)pUnused;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    ExitCallbackSig cb = (ExitCallbackSig)(pWrappedContext->fnCallback);
    cb(pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterExitCallback(ExitCallbackSig exitCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(exitCallback);
    CHECK_CLOUD_ENVIRONMENT();
    
    GFN_SDK_LOG("Registering for Exit Callback updates");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)exitCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterExitCallback, &_gfnExitCallbackWrapper, pWrappedContext);
}

static void GFN_CALLBACK _gfnPauseCallbackWrapper(int status, void* pUnused, void* pContext)
{
    (void)status;
    (void)pUnused;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    PauseCallbackSig cb = (PauseCallbackSig)(pWrappedContext->fnCallback);
    cb(pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(pauseCallback);
    CHECK_CLOUD_ENVIRONMENT();
    
    GFN_SDK_LOG("Registering for Pause Callback updates");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)pauseCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterPauseCallback, &_gfnPauseCallbackWrapper, pWrappedContext);
}

static void GFN_CALLBACK _gfnInstallCallbackWrapper(int status, void* pTitleInstallationInformation, void* pContext)
{
    (void)status;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    InstallCallbackSig cb = (InstallCallbackSig)(pWrappedContext->fnCallback);
    cb((TitleInstallationInformation*)pTitleInstallationInformation, pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterInstallCallback(InstallCallbackSig installCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(installCallback);
    CHECK_CLOUD_ENVIRONMENT();

    GFN_SDK_LOG("Registering for Install Callback updates");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)installCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterInstallCallback, &_gfnInstallCallbackWrapper, pWrappedContext);
}

static void GFN_CALLBACK _gfnSaveCallbackWrapper(int status, void* pUnused, void* pContext)
{
    (void)status;
    (void)pUnused;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    SaveCallbackSig cb = (SaveCallbackSig)(pWrappedContext->fnCallback);
    cb(pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterSaveCallback(SaveCallbackSig saveCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(saveCallback);
    CHECK_CLOUD_ENVIRONMENT();

    GFN_SDK_LOG("Registering for Save Callback updates");

    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)saveCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
    DELEGATE_TO_CLOUD_LIBRARY(RegisterSaveCallback, &_gfnSaveCallbackWrapper, pWrappedContext);
}

static void GFN_CALLBACK _gfnSessionInitCallbackWrapper(int status, void* pCString, void* pContext)
{
    (void)status;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    SessionInitCallbackSig cb = (SessionInitCallbackSig)(pWrappedContext->fnCallback);
    cb((const char *)pCString, pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterSessionInitCallback(SessionInitCallbackSig sessionInitCallback, void* pUserContext)
{
	CHECK_NULL_PARAM(sessionInitCallback);
    CHECK_CLOUD_ENVIRONMENT();

    GFN_SDK_LOG("Registering for SessionInit Callback updates");

	_gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
    pWrappedContext->fnCallback = (void*)sessionInitCallback;
    pWrappedContext->pOrigUserContext = pUserContext;

    // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
    // cppcheck-suppress memleak
	DELEGATE_TO_CLOUD_LIBRARY(RegisterSessionInitCallback, &_gfnSessionInitCallbackWrapper, pWrappedContext)
}

static void GFN_CALLBACK _gfnMessageCallbackWrapper(int status, void* pMessage, void* pContext)
{
    (void)status;
    _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)(pContext);
    if (pWrappedContext == NULL || pWrappedContext->fnCallback == NULL)
    {
        return;
    }
    MessageCallbackSig cb = (MessageCallbackSig)(pWrappedContext->fnCallback);
    cb((GfnString*)pMessage, pWrappedContext->pOrigUserContext);
}

GfnRuntimeError GfnRegisterMessageCallback(MessageCallbackSig messageCallback, void* pUserContext)
{
    CHECK_NULL_PARAM(messageCallback);

    if (g_pCloudLibrary != NULL && g_pCloudLibrary->RegisterMessageCallback != NULL)
    {
        _gfnUserContextCallbackWrapper* pWrappedContext = (_gfnUserContextCallbackWrapper*)malloc(sizeof(_gfnUserContextCallbackWrapper));
        pWrappedContext->fnCallback = (void*)messageCallback;
        pWrappedContext->pOrigUserContext = pUserContext;

        // Suppress CppCheck warning about memory leak. The cloud DLL takes ownership of the memory and will free on call to gfnShutdownSdk
        // cppcheck-suppress memleak
        DELEGATE_TO_CLOUD_LIBRARY(RegisterMessageCallback, &_gfnMessageCallbackWrapper, pWrappedContext);
    }
    else
    {
        if (g_gfnSdkModule == NULL)
        {
            return gfnAPINotInit;
        }

        gfnRegisterMessageCallbackFn fnRegisterMessageCallback = (gfnRegisterMessageCallbackFn)gfnGetSymbol(g_gfnSdkModule, "gfnRegisterMessageCallback");
        if (fnRegisterMessageCallback == NULL)
        {
            return gfnAPINotFound;
        }

        return fnRegisterMessageCallback(messageCallback, pUserContext);
    }
}


void gfnInitLogging()
{
#ifdef _WIN32
    wchar_t localAppDataPath[1024] = { L"" };
    if (SHGetSpecialFolderPathW(NULL, localAppDataPath, CSIDL_COMMON_APPDATA, false) == FALSE)
    {
        GFN_SDK_LOG("Could not get path to LOCALAPPDATA: %d", GetLastError());
        return;
    }
    wcscat_s(localAppDataPath, 1024, L"\\NVIDIA Corporation\\GfnRuntimeSdk");
    int createDirResult = SHCreateDirectoryExW(NULL, localAppDataPath, NULL);
    if (createDirResult != ERROR_SUCCESS && createDirResult != ERROR_FILE_EXISTS && createDirResult != ERROR_ALREADY_EXISTS)
    {
        return;
    }
    wcscat_s(localAppDataPath, 1024, L"\\GfnRuntimeSdkWrapper.log");
    _wfopen_s(&s_logfile, localAppDataPath, L"w+");
#elif __linux__
    const char* logPath = "~/.nvidia/GfnRuntimeSdk/GfnRuntimeSdkWrapper.log";
    s_logfile = fopen(logPath, "r");
#endif
}

void gfnDeinitLogging()
{
    if (s_logfile)
    {
        fclose(s_logfile);
        s_logfile = NULL;
    }
}

void gfnLog(char const* func, int line, char const* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef _WIN32
    // Format date and time
    GetLocalTime(&s_logData.timeBuffer);
    size_t n = sprintf_s(s_logData.buffer, 24, "%04d-%02d-%02dT%02d:%02d:%02d.%03d", s_logData.timeBuffer.wYear, s_logData.timeBuffer.wMonth, s_logData.timeBuffer.wDay,
        s_logData.timeBuffer.wHour, s_logData.timeBuffer.wMinute, s_logData.timeBuffer.wSecond, s_logData.timeBuffer.wMilliseconds);

    // Format function, line number
    n += _snprintf_s(s_logData.buffer + n, kGfnLogBufLen - n, kGfnLogBufLen - n, " %24.24s:%-5d", func, line);

    // Format the actual message/format
    n += vsnprintf(s_logData.buffer + n, kGfnLogBufLen - n - 2, format, args); // -1 leave room for linebreak and terminator
    if (kGfnLogBufLen - 2 < n)  // returns amount it WOULD have written if buffer were big enough
    {
        n = kGfnLogBufLen - 2;
    }
    _snprintf_s(s_logData.buffer + n, kGfnLogBufLen - n, kGfnLogBufLen - n, "\n"); // Add linebreak at end
#elif __linux__
    // Format date and time
    time_t t = time(NULL);
    localtime_r(&t, &s_logData.timeBuffer);

    size_t n = snprintf(s_logData.buffer, 24, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                        s_logData.timeBuffer.tm_year + 1900,
                        s_logData.timeBuffer.tm_mon + 1,
                        s_logData.timeBuffer.tm_mday,
                        s_logData.timeBuffer.tm_hour,
                        s_logData.timeBuffer.tm_min,
                        s_logData.timeBuffer.tm_sec,
                        0);

    // Format function, line number
    n += snprintf(s_logData.buffer + n, kGfnLogBufLen - n, " %24.24s:%-5d", func, line);

    // Format the actual message/format
    n += vsnprintf(s_logData.buffer + n, kGfnLogBufLen - n - 2, format, args);
    if (kGfnLogBufLen - 2 < n)  // returns amount it WOULD have written if buffer were big enough
    {
        n = kGfnLogBufLen - 2;
    }

    // Add linebreak at end
    snprintf(s_logData.buffer + n, kGfnLogBufLen - n, "\n");
#endif

    if (s_logfile)
    {
        fputs(s_logData.buffer, s_logfile);
        fflush(s_logfile);
    }
    else
    {
        fputs(s_logData.buffer, stderr);
        fflush(stderr);
    }
    va_end(args);
}
