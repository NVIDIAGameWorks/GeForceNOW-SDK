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

#include "gfn_sdk_demo/gfn_sdk_helper.h"

#include "include/cef_parser.h"
#include "shared/client_util.h"
#include "shared/defines.h"
#include "shared/main.h"
#include "GfnRuntimeSdk_Wrapper.h"  //Helper functions that wrap Library-based APIs
#include "shellapi.h"
#include <fstream>
#include "client.h"

CefString GFN_SDK_INIT = "GFN_SDK_INIT";
CefString GFN_SDK_SHUTDOWN = "GFN_SDK_SHUTDOWN";
CefString GFN_SDK_STREAM_ACTION = "GFN_SDK_STREAM_ACTION";
CefString GFN_SDK_IS_RUNNING_IN_CLOUD = "GFN_SDK_IS_RUNNING_IN_CLOUD";
CefString GFN_SDK_IS_RUNNING_IN_CLOUD_SECURE = "GFN_SDK_IS_RUNNING_IN_CLOUD_SECURE";
CefString GFN_SDK_GET_CLIENT_IP = "GFN_SDK_GET_CLIENT_IP";
CefString GFN_SDK_GET_CLIENT_COUNTRY_CODE = "GFN_SDK_GET_CLIENT_COUNTRY_CODE";
CefString GFN_SDK_GET_CLIENT_LANGUAGE_CODE = "GFN_SDK_GET_CLIENT_LANGUAGE_CODE";
CefString GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK = "GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK";
CefString GFN_SDK_IS_TITLE_AVAILABLE = "GFN_SDK_IS_TITLE_AVAILABLE";
CefString GFN_SDK_GET_AVAILABLE_TITLES = "GFN_SDK_GET_AVAILABLE_TITLES";
CefString GFN_SDK_REQUEST_ACCESS_TOKEN = "GFN_SDK_REQUEST_ACCESS_TOKEN";
CefString GET_TCP_PORT = "GET_TCP_PORT";
CefString GET_CLIENT_INFO = "GFN_SDK_GET_CLIENT_INFO";
CefString GFN_SDK_REGISTER_CLIENT_INFO_CALLBACK = "GFN_SDK_REGISTER_CLIENT_INFO_CALLBACK";
CefString GET_OVERRIDE_URI = "GET_OVERRIDE_URI";

static CefString DictToJson(CefRefPtr<CefDictionaryValue> dict)
{
    auto json = CefValue::Create();
    json->SetDictionary(dict);
    return CefWriteJSON(json, JSON_WRITER_DEFAULT);
}

static CefString GfnErrorToString(GfnError err)
{
    switch (err)
    {
    case GfnError::gfnSuccess: return "Success";
    case GfnError::gfnInitSuccessClientOnly: return "SDK initialization successful, but only client-side functionality available";
    case GfnError::gfnInitFailure: return "SDK initialization failure";
    case GfnError::gfnDllNotPresent: return "DLL Not Present";
    case GfnError::gfnComError: return "Com Error";
    case GfnError::gfnLibraryCallFailure: return "Error Calling Library Function";
    case GfnError::gfnIncompatibleVersion: return "Incompatible Version";
    case GfnError::gfnUnableToAllocateMemory: return "Unable To Allocate Memory";
    case GfnError::gfnInvalidParameter: return "Invalid Parameter";
    case GfnError::gfnInternalError: return "Internal Error";
    case GfnError::gfnUnsupportedAPICall: return "Unsupported API Call";
    case GfnError::gfnInvalidToken: return "Invalid Token";
    case GfnError::gfnTimedOut: return "Timed Out";
    case GfnError::gfnClientDownloadFailed: return "GFN Client download failed";
    case GfnError::gfnCallWrongEnvironment: return "Invalid cloud environment";
    case GfnError::gfnWebApiFailed: return "NVIDIA Web API returned invalid data";
    case GfnError::gfnStreamFailure: return "GFN Streamer hit a failure while starting a stream";
    case GfnError::gfnAPINotFound: return "GFN SDK API not found";
    case GfnError::gfnAPINotInit: return "GFN SDK API not initialized";
    case GfnError::gfnStreamStopFailure: return "GFN SDK failed to stop active streaming session";
    case GfnError::gfnCanceled: return "GFN SDK action was canceled";
    case GfnError::gfnElevationRequired: return "GFN SDK API requires process elevation";
    case GfnError::gfnThrottled: return "GFN SDK API cannot be called in rapid succession";
    case GfnError::gfnClientLibraryNotFound: return "GFN SDK API client library not found";
    case GfnError::gfnCloudLibraryNotFound: return "GFN SDK API cloud library not found";
    default: return "Unknown Error";
    }
}

static void logGfnSdkData()
{
    std::wstring appDataPath;
    shared::TryGetSpecialFolderPath(shared::SD_COMMONAPPDATA, appDataPath);
    std::wstring dataFilePath = appDataPath + L"\\NVIDIA Corporation\\GfnRuntimeSdk\\GFNSDK_Data.json";

    std::ifstream ifs(dataFilePath);
    if (!ifs)
    {
        LOG(ERROR) << "Could not locate " << dataFilePath;
    }
    else
    {
        std::string contents;

        ifs.seekg(0, std::ios::end);
        contents.reserve(ifs.tellg());
        ifs.seekg(0, std::ios::beg);

        contents.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        LOG(INFO) << "Contents of " << dataFilePath << ":";
        LOG(INFO) << contents;
        ifs.close();
    }
}

// Callback function for handling stream status callbacks
static void __stdcall handleStreamStatusCallback(GfnStreamStatus status, void* context);
static void __stdcall handleClientInfoCallback(GfnClientInfoUpdateData* pClientUpdate, void* context);
static CefMessageRouterBrowserSide::Callback* s_registerStreamStatusCallback = nullptr;
static CefMessageRouterBrowserSide::Callback* s_registerNetworkStatusCallback = nullptr;
static CefMessageRouterBrowserSide::Callback* s_registerClientInfoCallback = nullptr;

static GfnError initGFN()
{
    GfnError err = GfnInitializeSdk(GfnDisplayLanguage::gfnDefaultLanguage);
    switch (err)
    {
    case GfnError::gfnSuccess:
        LOG(INFO) << "succeeded at initializing GFNRuntime SDK";
        logGfnSdkData();
        break;
    case GfnError::gfnInitSuccessClientOnly:
        LOG(INFO) << "succeeded at initializing client-only GFNRuntime SDK";
        break;
    default:
        LOG(ERROR) << "error initializing: " << GfnErrorToString(err);
    }

    return err;
}

bool checkSampleServiceRunningStatus() {
    WCHAR* serviceName = L"GfnSdkSampleService";

    SC_HANDLE sch = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (sch == NULL)
    {
        LOG(INFO) << "OpenSCManager failed";
        return false;
    }

    SC_HANDLE svc = OpenService(sch, serviceName, SERVICE_QUERY_STATUS);
    if (svc == NULL)
    {
        LOG(INFO) << "OpenService failed";
        CloseServiceHandle(sch);
        return false;
    }

    bool serviceRunning = false;
    SERVICE_STATUS_PROCESS stat;
    DWORD needed = 0;
    BOOL ret = QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO,
        (BYTE*)&stat, sizeof stat, &needed);
    if (ret == TRUE)
    {
        if (stat.dwCurrentState == SERVICE_RUNNING)
        {
            serviceRunning = true;
            LOG(INFO) << serviceName << " is running";
        }
        else
        {
            LOG(INFO) << serviceName << " is NOT running";
        }
    }
    else
    {
        LOG(INFO) << "QueryServiceStatusEx failed";
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(sch);

    return serviceRunning;
}

bool GfnSdkHelper(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int64 query_id,
    const CefString& request,
    bool persistent,
    CefRefPtr<CefMessageRouterBrowserSide::Callback> callback)
{
    CefRefPtr<CefValue> requestValue = CefParseJSON(request, JSON_PARSER_RFC);

    CefRefPtr<CefDictionaryValue> dict = requestValue->GetDictionary();
    CefString command = dict->GetString("command");

    /**
     * Query command for initializing the GFN SDK. Should be called once during launcher startup
     * before making any other GFN SDK calls.
     */
    if (command == GFN_SDK_INIT)
    {
        GfnError err = initGFN();
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        // gfnInitSuccessClientOnly is also success, and needs to be treated as such
        response_dict->SetBool("success", (err == GfnError::gfnSuccess || err == GfnError::gfnInitSuccessClientOnly));
        response_dict->SetString("errorMessage", GfnErrorToString(err));

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }
    /**
     * Query command for shutting down the GFN SDK. Should be called once the SDK is no longer needed.
     * Manually wire up into gfn_sdk.html where you want shutdown to occur.
     */
    if (command == GFN_SDK_SHUTDOWN)
    {
        GfnError err = GfnShutdownSdk();
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetBool("success", (err == GfnError::gfnSuccess));
        response_dict->SetString("errorMessage", GfnErrorToString(err));

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }
    /**
     * Calls into GFN SDK to determine whether the sample launcher is being executed inside
     * an NVIDIA GeForce NOW game seat.
     */
    else if (command == GFN_SDK_IS_RUNNING_IN_CLOUD)
    {
        bool enabled = false;
        GfnError err = GfnIsRunningInCloud(&enabled);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "Failed to get if running in cloud. Error: " << err;
            return true;
        }

        LOG(INFO) << "is enabled: " << enabled;

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetBool("enabled", enabled);

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    /**
    * Calls into GFN SDK securely to determine whether the sample launcher is being executed inside
    * an NVIDIA GeForce NOW game seat. Call will only succeed if the sample launcher was executed
    * as an elevated process. In a real-world implementation, this would be called via a separate
    * OS-based/elevated process via an IPC mechanism.
    */
    else if (command == GFN_SDK_IS_RUNNING_IN_CLOUD_SECURE)
    {
        GfnIsRunningInCloudAssurance assurance = GfnIsRunningInCloudAssurance::gfnNotCloud;
        CefString errorMessage;

        bool isServiceRunning = checkSampleServiceRunningStatus();
        if (isServiceRunning)
        {
            SampleService::ServiceClient client;
            const auto [status, gfnstatus, value] = client.isRunningInCloudSecure();
            if (status == SampleService::status::success)
            {
                GfnError err = static_cast<GfnError>(std::stoi(gfnstatus));
                if (err == GfnError::gfnSuccess)
                {
                    assurance = static_cast<GfnIsRunningInCloudAssurance>(std::stoi(value));
                }
                else
                {
                    LOG(ERROR) << "Failed to get if running in cloud. Error: " << err;
                }
                errorMessage = GfnErrorToString(err);
            }
            else
            {
                errorMessage = "ServiceClient error: " + std::to_string(static_cast<int>(status));
            }
        }
        else
        {
            errorMessage = "GfnSdkSampleService is not running";
        }

        LOG(INFO) << "Cloud environment assurance value: " << assurance;

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetInt("assurance", assurance);
        response_dict->SetString("errorMessage", errorMessage);

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    /**
     * Calls into GFN SDK to determine if a specific title is available to stream right now
     * directly from the NVIDIA GeForce NOW game seat.
     */
    else if (command == GFN_SDK_IS_TITLE_AVAILABLE)
    {
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        if (dict->HasKey("appId"))
        {
            std::string pchappId = dict->GetString("appId").ToString();
            bool available = false;
            GfnError err = GfnIsTitleAvailable(pchappId.c_str(), &available);
            if (err != GfnError::gfnSuccess)
            {
                LOG(ERROR) << "is title available error: " << GfnErrorToString(err);
            }
            else
            {
                LOG(INFO) << "is available to stream: " << available;
            }

            response_dict = CefDictionaryValue::Create();
            response_dict->SetBool("available", available);
        }
        else
        {
            response_dict->SetString("errorMessage", CefString("Bad arguments to CEF extension"));
        }

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }
    /**
     * Calls into GFN SDK to determine all titles available to stream right now directly from the
     * NVIDIA GeForce NOW game seat.
     */
    else if (command == GFN_SDK_GET_AVAILABLE_TITLES)
    {
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        char const* appIds = nullptr;
        GfnError err = GfnGetTitlesAvailable(&appIds);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get available titles error: " << GfnErrorToString(err);
            response_dict->SetString("titles", "");
        }
        else
        {
            response_dict->SetString("titles", appIds);
            GfnFree(&appIds);
        }

        response_dict->SetString("errorMessage", GfnErrorToString(err));
        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    /**
     * Calls into GFN SDK to initiate a new GeForce NOW streaming session and launch the specified
     * game application. If a valid delegate token is passed in as a parameter it will automatically
     * authenticate the streaming session and begin streaming immediately. If an empty or invalid
     * delegate token is passed in then the GeForce NOW streaming client will display a login
     * window prompting the user to authenticate first.
     */
    else if (command == GFN_SDK_STREAM_ACTION)
    {
        bool actionSuccess = false;
        bool launchStream = false;
        std::string msg;
        if (!dict->HasKey("launchStream"))
        {
            msg = "Call to Stream action missing \"launchStream\" parameter";
        }
        launchStream = dict->GetBool("launchStream");
        if (launchStream)
        {
            LOG(INFO) << "Received request to start a session";
            if (dict->HasKey("gfnTitleId") && dict->HasKey("authToken") && dict->HasKey("tokenType"))
            {
                StartStreamResponse response = { 0 };
                StartStreamInput startStreamInput = { 0 };
                uint32_t gfnTitleId = dict->GetInt("gfnTitleId");
                startStreamInput.uiTitleId = gfnTitleId;

                // NVIDIA IDM authorization token (when NV user has logged in)
                std::string pchAuthToken = dict->GetString("authToken").ToString();
                startStreamInput.pchAuthToken = pchAuthToken.c_str();
                bool hasTokenType = false;
                std::stringstream ssTokenType(dict->GetString("tokenType"));
                ssTokenType >> startStreamInput.tokenType;
                hasTokenType = !ssTokenType.fail() && !ssTokenType.bad();

                // 3rd party IDM token for SSO that is consumed by launcher application running in GFN
                std::string pchcustAuth = dict->GetString("launcherToken").ToString();
                startStreamInput.pchCustomAuth = pchcustAuth.c_str();

                if (hasTokenType)
                {
                    startStreamInput.pchCustomData = "This is example custom data";

                    GfnError err = GfnStartStream(&startStreamInput, &response);
                    msg = "gfnStartStream = " + std::string(GfnErrorToString(err));
                    if (err != GfnError::gfnSuccess)
                    {
                        LOG(ERROR) << "launch game error: " << msg;
                    }
                    else
                    {
                        actionSuccess = true;
                        msg = msg + ", GFN Downloaded & Installed = " + (response.downloaded ? "Yes" : "Not needed");
                        LOG(INFO) << "launch game response. Downloaded GeForceNOW? : " << response.downloaded;
                    }
                }
                else
                {
                    msg = "An error occurred while parsing tokenType argument to CEF extention";
                }
            }
            else
            {
                msg = "Bad arguments to CEF extension";
            }
        }
        else
        {
            LOG(INFO) << "Received request to stop a session";
            GfnError err = GfnStopStream();
            msg = "gfnStopStream = " + std::string(GfnErrorToString(err));
            if (err != GfnError::gfnSuccess)
            {
                LOG(ERROR) << "Stream stop error: " << msg;
            }
            else
            {
                actionSuccess = true;
                LOG(INFO) << "Stream stop success";
            }
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetBool("actionSuccess", actionSuccess);
        response_dict->SetString("errorMessage", CefString(msg.c_str()));

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }
    /**
     * Calls into GFN SDK to get the user's current local IP address. This is meant to be
     * called while running on the GeForce NOW game seat to handle scenarios where the IP
     * addressed is used for account security or other validation purposes or things like
     * region specific localization or other UI.
     */
    else if (command == GFN_SDK_GET_CLIENT_IP)
    {
        char const* clientIp = nullptr;
        GfnError err = GfnGetClientIpV4(&clientIp);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get client IP error: " << GfnErrorToString(err);
        }
        else
        {
            LOG(INFO) << "client ip: " << clientIp;
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("clientIp", clientIp);
        response_dict->SetString("errorMessage", GfnErrorToString(err));

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    /**
     * Calls into GFN SDK to get the user's country code. This is meant to be
     * called while running on the GeForce NOW game seat to determine the country where
     * user is located
     */
    else if (command == GFN_SDK_GET_CLIENT_COUNTRY_CODE)
    {
        char clientCountryCode[3] = { 0 };
        GfnError err = GfnGetClientCountryCode(clientCountryCode, 3);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get client country code error: " << GfnErrorToString(err);
        }
        else
        {
            LOG(INFO) << "client country code: " << clientCountryCode;
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("clientCountryCode", clientCountryCode);
        response_dict->SetString("errorMessage", GfnErrorToString(err));

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }

    /**
     * Calls into GFN SDK to get the user's language code. THis is meant to be
     * called while running on the GeForce NOW game seat to determine the language
     * used by the user.
     */
    else if (command == GFN_SDK_GET_CLIENT_LANGUAGE_CODE)
    {
        const char* clientLanguageCode;
        GfnError err = GfnGetClientLanguageCode(&clientLanguageCode);
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get client country code error: " << GfnErrorToString(err);
            response_dict->SetString("clientLanguageCode", "");
            response_dict->SetString("errorMessage", GfnErrorToString(err));
        }
        else
        {
            LOG(INFO) << "client country code: " << clientLanguageCode;
            response_dict->SetString("clientLanguageCode", clientLanguageCode);
            response_dict->SetString("errorMessage", "");
        }

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        if (err == GfnError::gfnSuccess)
        {
            GfnFree(&clientLanguageCode);
        }
        return true;
    }

    /**
     * Registers for callback notifications during a streaming session.
     */
    else if (command == GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK)
    {
        s_registerStreamStatusCallback = callback;

        GfnError err = GfnRegisterStreamStatusCallback(reinterpret_cast<StreamStatusCallbackSig>(&handleStreamStatusCallback), nullptr);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "Failed to register Stream Status Callback: " << GfnErrorToString(err);
        }
        return true;
    }
    /**
     * Requests a copy of the token data that was passed into customAuth as part of the call to one
     * of the StartStream APIs. This call should only be made in the GFN cloud environment.
     */
    else if (command == GFN_SDK_REQUEST_ACCESS_TOKEN)
    {
        char const* authData = nullptr;
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        GfnError err = GfnGetAuthData(&authData);
        response_dict->SetString("errorMessage", GfnErrorToString(err));
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get authorization data error: " << GfnErrorToString(err);
            response_dict->SetString("authData", "");
        }
        else
        {
            response_dict->SetString("authData", authData);
            GfnFree(&authData);
        }

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    else if (command == GET_TCP_PORT)
    {
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("port", shared::g_activePort);

        CefString response(DictToJson(response_dict));
        callback->Success(response);

        return true;
    }
    else if (command == GET_CLIENT_INFO)
    {
        LOG(INFO) << "Calling GfnGetClientInfo...";
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        GfnClientInfo clientInfo = { 0 };
        GfnError error = GfnError::gfnSuccess;
        error = GfnGetClientInfo(&clientInfo);
        LOG(INFO) << "GfnGetClientInfo error result: " << error;
        response_dict->SetString("errorMessage", GfnErrorToString(error));
        if (error != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "get client info data error: " << GfnErrorToString(error);
            response_dict->SetString("clientInfo", "");
        }
        else
        {
            response_dict->SetInt("apiVersion", clientInfo.version);
            response_dict->SetString("country", clientInfo.country);
            response_dict->SetString("ipV4", clientInfo.ipV4);
            response_dict->SetString("locale", clientInfo.locale);
            response_dict->SetInt("osType", clientInfo.osType);
        }
        CefString response(DictToJson(response_dict));
        LOG(INFO) << "GfnGetClientInfo data: " << response.ToString();
        callback->Success(response);
        return true;
    }
    /**
     * Registers for callback notifications for on-seat client info updates
     */
    else if (command == GFN_SDK_REGISTER_CLIENT_INFO_CALLBACK)
    {
        s_registerClientInfoCallback = callback;

        GfnError err = GfnRegisterClientInfoCallback(reinterpret_cast<ClientInfoCallbackSig>(&handleClientInfoCallback), nullptr);
        if (err != GfnError::gfnSuccess)
        {
            LOG(ERROR) << "Failed to register Client Info Callback: " << GfnErrorToString(err);
        }
        return true;
    }
    else if (command == GET_OVERRIDE_URI)
    {
        int argc = 0;
        LPWSTR* cmdLine = CommandLineToArgvW(GetCommandLineW(), &argc);
        LPWSTR override_uri = L"";
        for (int i = 0; i < argc; i++)
        {
            if (wcscmp(cmdLine[i], L"--override_uri") == 0)
            {
                if (i + 1 < argc)
                {
                    override_uri = cmdLine[i + 1];
                }
                break;
            }
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("overrideURI", override_uri);

        CefString response(DictToJson(response_dict));
        LOG(INFO) << "Override URI: " << response.ToString();
        callback->Success(response);
        return true;
    }
    LOG(ERROR) << "Unknown command value: " << command;

    return false;
}

void __stdcall handleStreamStatusCallback(GfnStreamStatus status, void* context)
{
    if (s_registerStreamStatusCallback)
    {
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("status", GfnStreamStatusToString(status));

        CefString response(DictToJson(response_dict));
        s_registerStreamStatusCallback->Success(response);
    }
}

void __stdcall handleClientInfoCallback(GfnClientInfoUpdateData* pClientUpdate, void* context)
{
    if (!pClientUpdate)
    {
        return;
    }
    if (s_registerClientInfoCallback)
    {
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        switch (pClientUpdate->updateType)
        {
        case gfnOs:
            response_dict->SetInt("os", pClientUpdate->data.osType);
            break;
        default:
            return;
        }

        CefString response(DictToJson(response_dict));
        s_registerClientInfoCallback->Success(response);
    }
}
