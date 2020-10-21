// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#include "gfn_sdk_demo/gfn_sdk_helper.h"

#include "include/cef_parser.h"
#include "shared/client_util.h"
#include "shared/defines.h"
#include "shared/main.h"
#include "GfnRuntimeSdk_Wrapper.h"  //Helper functions that wrap Library-based APIs

#include <fstream>

CefString GFN_SDK_INIT = "GFN_SDK_INIT";
CefString GFN_SDK_STREAM_ACTION = "GFN_SDK_STREAM_ACTION";
CefString GFN_SDK_IS_RUNNING_IN_CLOUD = "GFN_SDK_IS_RUNNING_IN_CLOUD";
CefString GFN_SDK_GET_CLIENT_IP = "GFN_SDK_GET_CLIENT_IP";
CefString GFN_SDK_GET_CLIENT_COUNTRY_CODE = "GFN_SDK_GET_CLIENT_COUNTRY_CODE";
CefString GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK = "GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK";
CefString GFN_SDK_IS_TITLE_AVAILABLE = "GFN_SDK_IS_TITLE_AVAILABLE";
CefString GFN_SDK_GET_AVAILABLE_TITLES = "GFN_SDK_GET_AVAILABLE_TITLES";
CefString GFN_SDK_REQUEST_ACCESS_TOKEN = "GFN_SDK_REQUEST_ACCESS_TOKEN";
CefString GET_TCP_PORT = "GET_TCP_PORT";

static CefString DictToJson(CefRefPtr<CefDictionaryValue> dict)
{
    auto json = CefValue::Create();
    json->SetDictionary(dict);
    return CefWriteJSON(json, JSON_WRITER_DEFAULT);
}

static CefString GfnRuntimeErrorToString(GfnRuntimeError err)
{
    switch (err)
    {
    case GfnRuntimeError::gfnSuccess: return "Success";
    case GfnRuntimeError::gfnInitSuccessClientOnly: return "SDK initialization successful, but only client-side functionality available";
    case GfnRuntimeError::gfnInitFailure: return "SDK initialization failure";
    case GfnRuntimeError::gfnDllNotPresent: return "DLL Not Present";
    case GfnRuntimeError::gfnComError: return "Com Error";
    case GfnRuntimeError::gfnLibraryCallFailure: return "Error Calling Library Function";
    case GfnRuntimeError::gfnIncompatibleVersion: return "Incompatible Version";
    case GfnRuntimeError::gfnUnableToAllocateMemory: return "Unable To Allocate Memory";
    case GfnRuntimeError::gfnInvalidParameter: return "Invalid Parameter";
    case GfnRuntimeError::gfnInternalError: return "Internal Error";
    case GfnRuntimeError::gfnUnsupportedAPICall: return "Unsupported API Call";
    case GfnRuntimeError::gfnInvalidToken: return "Invalid Token";
    case GfnRuntimeError::gfnTimedOut: return "Timed Out";
    case GfnRuntimeError::gfnClientDownloadFailed: return "GFN Client download failed";
    case GfnRuntimeError::gfnWebApiFailed: return "NVIDIA Web API returned invalid data";
    case GfnRuntimeError::gfnStreamFailure: return "GFN Streamer hit a failure while starting a stream";
    case GfnRuntimeError::gfnAPINotFound: return "GFN SDK API not found";
    case GfnRuntimeError::gfnAPINotInit: return "GFN SDK API not initialized";
    case GfnRuntimeError::gfnStreamStopFailure: return "GFN SDK failed to stop active streaming session";
    case GfnRuntimeError::gfnCanceled: return "GFN SDK action was canceled";
    default: return "Unknown Error";
    }
}

static void logGfnSdkData()
{
    std::wstring appDataPath;
    shared::TryGetSpecialFolderPath(shared::SD_LOCALAPPDATA, appDataPath);
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
static CefMessageRouterBrowserSide::Callback* s_registerStreamStatusCallback = nullptr;

static GfnRuntimeError initGFN()
{
    GfnRuntimeError err = GfnInitializeSdk(GfnDisplayLanguage::gfnDefaultLanguage);
    switch (err)
    {
    case GfnRuntimeError::gfnSuccess:
        LOG(INFO) << "succeeded at initializing GFNRuntime SDK";
        logGfnSdkData();
        break;
    case GfnRuntimeError::gfnInitSuccessClientOnly:
        LOG(INFO) << "succeeded at initializing client-only GFNRuntime SDK";
        break;
    default:
        LOG(ERROR) << "error initializing: " << GfnRuntimeErrorToString(err);
    }

    return err;
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
        GfnRuntimeError err = initGFN();
        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        // gfnInitSuccessClientOnly is also success, and needs to be treated as such
        response_dict->SetBool("success", (err == GfnRuntimeError::gfnSuccess || err == GfnRuntimeError::gfnInitSuccessClientOnly));
        response_dict->SetString("errorMessage", GfnRuntimeErrorToString(err));

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
        GfnRuntimeError err = GfnIsRunningInCloud(&enabled);
        if (err != GfnRuntimeError::gfnSuccess)
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
            GfnRuntimeError err = GfnIsTitleAvailable(pchappId.c_str(), &available);
            if (err != GfnRuntimeError::gfnSuccess)
            {
                LOG(ERROR) << "is title available error: " << GfnRuntimeErrorToString(err);
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
        GfnRuntimeError err = GfnGetTitlesAvailable(&appIds);
        if (err != GfnRuntimeError::gfnSuccess)
        {
            LOG(ERROR) << "get available titles error: " << GfnRuntimeErrorToString(err);
            response_dict->SetString("titles", "");
        }
        else
        {
            response_dict->SetString("titles", appIds);
            GfnFree(&appIds);
        }

        response_dict->SetString("errorMessage", GfnRuntimeErrorToString(err));
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

                    GfnRuntimeError err = GfnStartStream(&startStreamInput, &response);
                    msg = "gfnStartStream = " + std::string(GfnRuntimeErrorToString(err));
                    if (err != GfnRuntimeError::gfnSuccess)
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
            GfnRuntimeError err = GfnStopStream();
            msg = "gfnStopStream = " + std::string(GfnRuntimeErrorToString(err));
            if (err != GfnRuntimeError::gfnSuccess)
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
        GfnRuntimeError err = GfnGetClientIpV4(&clientIp);
        if (err != GfnRuntimeError::gfnSuccess)
        {
            LOG(ERROR) << "get client IP error: " << GfnRuntimeErrorToString(err);
        }
        else
        {
            LOG(INFO) << "client ip: " << clientIp;
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("clientIp", clientIp);
        response_dict->SetString("errorMessage", GfnRuntimeErrorToString(err));

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
        GfnRuntimeError err = GfnGetClientCountryCode(clientCountryCode, 3);
        if (err != GfnRuntimeError::gfnSuccess)
        {
            LOG(ERROR) << "get client country code error: " << GfnRuntimeErrorToString(err);
        }
        else
        {
            LOG(INFO) << "client country code: " << clientCountryCode;
        }

        CefRefPtr<CefDictionaryValue> response_dict = CefDictionaryValue::Create();
        response_dict->SetString("clientCountryCode", clientCountryCode);
        response_dict->SetString("errorMessage", GfnRuntimeErrorToString(err));

        CefString response(DictToJson(response_dict));
        callback->Success(response);
        return true;
    }
    /**
     * Registers for callback notifications during a streaming session.
     */
    else if (command == GFN_SDK_REGISTER_STREAM_STATUS_CALLBACK)
    {
        s_registerStreamStatusCallback = callback;

        GfnRuntimeError err = GfnRegisterStreamStatusCallback(reinterpret_cast<StreamStatusCallbackSig>(&handleStreamStatusCallback), nullptr);
        if (err != GfnRuntimeError::gfnSuccess)
        {
            LOG(ERROR) << "Failed to register Stream Status Callback: " << GfnRuntimeErrorToString(err);
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
        GfnRuntimeError err = GfnGetAuthData(&authData);
        response_dict->SetString("errorMessage", GfnRuntimeErrorToString(err));
        if (err != GfnRuntimeError::gfnSuccess)
        {
            LOG(ERROR) << "get authorization data error: " << GfnRuntimeErrorToString(err);
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
