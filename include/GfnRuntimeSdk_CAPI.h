// ===============================================================================================
//
/* Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

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
//
// ===============================================================================================
//
// C / C++ API declarations for GFN Runtime SDK
//
// ===============================================================================================
/**
 * @file GfnRuntimeSdk_CAPI.h
 *
 * API definitions
 */
///
/// @mainpage Native Runtime API Reference
///
/// @section introduction Introduction
/// The NVIDIA GFN Runtime SDK provides a set of interfaces to allow game developers and game
/// publishers to interact with parts of the NVIDIA GeForce NOW ecosystem. Integration is provided
/// by various means, from native C interfaces to RESTful Web API calls, depending on the feature
/// of the SDK.
///
/// @image html ecosystem.png
///
/// This document provides details of how to integrate the native APIs of GFN Runtime SDK features
/// into your application and its developer and deployment processes. For more detailed technical
/// information about the architecture, refer to the architecture section later in this document
///
/// @section overview Overview
///
/// @image html architecture.png
///
/// The GFN Runtime SDK provides a static C  library that is linked to the game/application and
/// exposes the various APIs defined in this document. The behavior of those APIs depends on the
/// environment the application is running in; either on a client/user system or in the GeForce
/// NOW (GFN) cloud environment.
///
/// On client systems, this library checks for the presence of the GeForce NOW (GFN) client
/// installation at initialization time. If the client is not present, or the client is out of date,
/// then the library will initiate a download and installation of the latest client on the first
/// API call that requires the client to be present, presenting UI to the user for the
/// installation process.
///
/// If the GFN client is installed on the system, the static library will defer API actions to
/// components in the installation.
///
/// This design allows the static library to stay as thin as possible in the application, and
/// provides backward and forward compatibility to new GFN client packages.
///
/// On GFN game systems, many of the APIs are no - ops as they apply only to client/end-user
/// systems. In those cases, API calls will return a well - defined error code to denote the call
/// was not applicable to the environment.
///
/// NVIDIA provides a backend IDM Service that application developers can use to validate users
/// and obtain user information from.This provides a seamless flow for users to bypass multiple
/// login steps in streaming a game through GFN.
///
/// A simplified flow diagram of how this functions is shown below:
///
/// @image html authentication.png
///
/// Authentication between the Partner IDM and NVIDIA IDM happens using secure
/// HTTPS web API calls, and the account linking flow utilizes standard oauth2
/// protocol. Once the account link is established the authentication process
/// between Partner and NVIDIA becomes transparent to the user, and gaming
/// streaming can be initiated without requiring any further authentication or
/// manual login.
///
/// @image html account_link_flow.png
///
/// After the account link between Partner and NVIDIA has been established, that
/// link can be utilized on the GFN server to facilitate Single Sign-On (SSO) so
/// that the user does not have to manually login again, but all authentication
/// happens transparently and the game launches immediately.
///
/// @image html sso.png
///
/// For additional high-level overview, please refer to the SDK primer available as part of the
/// documentation section of the SDK’s repository.
///
/// @section keyconcepts Key Concepts
///
/// GFN Runtime API methods are used to make requests from or to notify the GFN
/// backend.
///
/// When your application is operating outside of the GFN environment, these
/// methods are simple stubs that incur almost no cost, so it's safe to add
/// these to your main build.
///
/// The calling convention differs by which API you've selected to use. In most
/// cases, the methods return a GfnRuntimeError result, which can be used by the
/// application to check for errors. In addition, some methods are asynchronous
/// by nature, but provide synchronous variants when possible.
///
/// @image html sequence.png
///
/// @section apiReference API Reference
/// @subsection general_section General / Common methods
/// @ref general
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnInitializeRuntimeSdk
///
/// @copydoc gfnInitializeRuntimeSdk
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnShutdownRuntimeSdk
///
/// @copydoc gfnShutdownRuntimeSdk
///
/// @subsection launcher_game_section Launcher or Game Specific Methods
/// @ref launcher
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnIsRunningInCloud
///
/// @copydoc gfnIsRunningInCloud
///
/// Language | API
/// -------- | -------------------------------------
/// C sync   | @ref gfnSetupTitle
///
/// @copydoc gfnSetupTitle
///
/// Language | API
/// -------- | -------------------------------------
/// C sync   | @ref gfnTitleExited
///
/// @copydoc gfnTitleExited
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnGetClientIp
///
/// @copydoc gfnGetClientIp
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnGetClientLanguageCode
///
/// @copydoc gfnGetClientLanguageCode
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnRequestGfnAccessToken
///
/// @copydoc gfnRequestGfnAccessToken
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnStartStream
///
/// @copydoc gfnStartStream
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnStartStreamAsync
///
/// @copydoc gfnStartStreamAsync
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnGetCustomData
///
/// @copydoc gfnGetCustomData
///
/// @subsection callback_section Callback-related Methods
/// @ref callbacks
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnRegisterExitCallback
///
/// @copydoc gfnRegisterExitCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnRegisterPauseCallback
///
/// @copydoc gfnRegisterPauseCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnRegisterInstallCallback
///
/// @copydoc gfnRegisterInstallCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnRegisterStreamStatusCallback
///
/// @copydoc gfnRegisterStreamStatusCallback
///

#ifndef GFN_RUNTIME_SDK_CAPI_H
#define GFN_RUNTIME_SDK_CAPI_H

// Required for streamer / DLL download step in StartStream
#ifdef _WIN32
#   pragma comment(lib, "ws2_32.lib")
#   pragma comment(lib, "crypt32.lib")
#   pragma comment(lib, "Wldap32.lib")
#   pragma comment(lib, "version.lib")
#   pragma comment(lib, "wintrust")
#   define GFN_CALLBACK __stdcall
#else
#   define GFN_CALLBACK
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
typedef char bool;
#define false 0
#define true 1
#endif

#define GFN_RUNTIME_SDK_VERSION 1.2

/// One of the possible values of AuthType_t. Delegate token provided by NVIDIA IDM
#define AUTH_JARVIS 7  
/// One of the possible values of AuthType_t. JSON Web Token
#define AUTH_JWT 8

#ifdef __cplusplus
namespace GfnRuntimeSdk
{
    extern "C"
    {
#endif
        // ============================================================================================
        // Constants/Enums
        // ============================================================================================

        /// @brief Returned by InitializeGfnRuntime and GfnRuntime API methods
        typedef enum GfnRuntimeError
        {
            gfnSuccess =                  0,
            gfnInitSuccessClientOnly =    1, ///< SDK initialized, but only cloud independent functionality available (such as gfnStartStream).
            gfnInitFailure           =   -1, ///< SDK initialization failure for any reason other than memory allocation failure.
            gfnDllNotPresent =           -2,
            gfnComError =                -3, ///< Geforce NOW SDK internal component communication error.
            gfnErrorCallingDLLFunction = -4, ///< Geforce NOW SDK components were reachable, but could not serve the request.
            gfnIncompatibleVersion =     -5,
            gfnUnableToAllocateMemory =  -6,
            gfnInvalidParameter =        -7,
            gfnInternalError =           -8, ///< Generic Geforce NOW SDK internal error.
            gfnUnsupportedAPICall =      -9,
            gfnInvalidToken =            -10,
            gfnTimedOut =                -11,
            gfnSetupTitleFailure =       -12,
            gfnClientDownloadFailed =    -13, ///< Failed to download the Geforce NOW client.
            gfnCallWrongEnvironment =    -14, ///< Function limited to specific environment called in wrong environment
            gfnWebApiFailed          =   -15, ///< A call to a NVIDIA Web API failed to return valid data
            gfnStreamFailure =           -16  ///< GeForceNOW Streamer hit a failure while starting a stream
        } GfnRuntimeError;

        /// @brief Values for languages supported by the GFN SDK, used to define which language any SDK dialogs should be displayed in.
        typedef enum GfnDisplayLanguage
        {
            gfnDefaultLanguage = 0,         /// Uses the default system language
            gfn_bg_BG = 1,
            gfn_cs_CZ = 2,
            gfn_da_DK = 2,
            gfn_nl_NL = 3,
            gfn_de_DE = 4,
            gfn_el_GR = 5,
            gfn_en_US = 6,
            gfn_en_UK = 7,
            gfn_es_ES = 8,
            gfn_es_MX = 9,
            gfn_fi_FI = 10,
            gfn_fr_FR = 11,
            gfn_hu_HU = 12,
            gfn_it_IT = 13,
            gfn_ja_JA = 14,
            gfn_ko_KR = 15,
            gfn_nb_NO = 16,
            gfn_po_PO = 17,
            gfn_pt_BR = 18,
            gfn_pt_PT = 19,
            gfn_ro_RO = 20,
            gfn_ru_RU = 21,
            gfn_sv_SE = 22,
            gfn_th_TH = 23,
            gfn_tr_TR = 24,
            gfn_uk_UA = 25,
            gfn_zh_CN = 26,
            gfn_zh_TW = 27,
            gfnMaxLanguage = gfn_zh_TW
        } GfnDisplayLanguage;

        /* @brief Returned by callbacks the application registers with the Geforce NOW Runtime SDK, or passes
         *  in to asynchronous SDK calls. */
        typedef enum GfnApplicationCallbackResult
        {
            crCallbackSuccess =  0, ///< Return to indicate that a callback has performed the requested operation
            crCallbackFailure = -1  ///< Return to indicate that a callback did not perform the requested operation
        } GfnApplicationCallbackResult;

        /// @brief Output response when streaming has started
        typedef struct StartStreamResponse
        {
            bool downloaded; ///< True if Geforce NOW client components were downloaded from the release site.
        } StartStreamResponse;

        /// @brief Callback function signation for notifications on status of starting a streaming session.
        typedef void(GFN_CALLBACK *StartStreamCallbackSig)(GfnRuntimeError, StartStreamResponse*, void* context);

        /// @brief Type of token to use for GFN session. Valid values include AUTH_JARVIS and AUTH_JWT.
        typedef unsigned int AuthType_t;

        /// @brief Input data for gfnStartStream
        typedef struct StartStreamInput
        {
            unsigned int uiTitleId;     ///< GFN-sourced game-specific unique identifier.
            const char* pchAuthToken;   ///< Token string
            AuthType_t tokenType;    ///< Token identifier
            const char* pchCustomData;  ///< Optional data that is passed to the streaming cloud instance and can be retrieved in that instance of application.
                                        ///< NOTE: Will be implemented in next release.
            const char* pchCustomAuth;  ///< Optional client-specific data to authenticate the user on the game seat.
        } StartStreamInput;

        /// @brief Input to the function registered via gfnRegisterInstallCallback (if any).
        typedef struct TitleInstallationInformation
        {
            const char* pchPlatformAppId; ///< Platform application ID passed into the API.
            const char* pchBuildPath;     ///< The path at which game build files can be found.
            const char* pchMetadataPath;  ///< Optionally contains the path at which game metadata can be found, else NULL.
        } TitleInstallationInformation;

        /// @brief Possible states of the client streamer
        typedef enum GfnStreamStatus
        {
            Init,           ///< Initial default state
            NetworkTest,    ///< Client is running a network test to determine the best zone
            Loading,        ///< Client is loading the game
            Streaming,      ///< Client is actively streaming
            Done,           ///< Client has successfully finished streaming
            Error           ///< Client has stopped streaming and has entered an error state
        } GfnStreamStatus;

        /// @brief Converts a GfnStreamStatus enum to a human-readable string
        inline const char* GfnStreamStatusToString(GfnStreamStatus status)
        {
            switch (status)
            {
            case Init:          return "Init";
            case NetworkTest:   return "NetworkTest";
            case Loading:       return "Loading";
            case Streaming:     return "Streaming";
            case Done:          return "Done";
            case Error:         return "Error";
            }
            return "Unknown GfnStreamStatus";
        }

        // ============================================================================================
        // Callback signatures
        // ============================================================================================

        // Callbacks that client code should implement and register via the appropriate callback
        // registration function (listed as part of the C API below).
        /// @brief Callback function for notification when a game should exit. Register via gfnRegisterExitCallback API.
        typedef GfnApplicationCallbackResult(GFN_CALLBACK *ExitCallbackSig)(void* pUserContext);
        /// @brief Callback function for notification when a game should pause. Register via gfnRegisterPauseCallback API.
        typedef GfnApplicationCallbackResult(GFN_CALLBACK *PauseCallbackSig)(void* pUserContext);
        /// @brief Callback function for notification when a game is being installed to allow for installation actions. Register via gfnRegisterInstallCallback API.
        typedef GfnApplicationCallbackResult(GFN_CALLBACK *InstallCallbackSig)(const TitleInstallationInformation* pInfo, void* pUserContext);
        /// @brief Callback function for notifications on status of starting a streaming session. Register via gfnRegisterStreamStatusCallback API.
        typedef GfnApplicationCallbackResult(GFN_CALLBACK *StreamStatusCallbackSig)(GfnStreamStatus status, void* pUserContext);

        // ============================================================================================
        // C API
        // ============================================================================================

        // ============================================================================================
        // SDK Initialization / Shutdown

        /// @defgroup general General / Common Methods
        /// The functions contained in this section control the lifecycle of the SDK.
        /// @{

        /// @par Description
        /// Should be called at application startup and prior to any other GFN Runtime API methods.
        /// The SDK features which become available as a result of initializing the Geforce NOW
        /// SDK are dependent on the type of environment in which the SDK operates (client or cloud).
        ///
        /// @par Environment
        /// Cloud and Client
        ///
        /// @par Usage
        /// Call as soon as possible during application startup.
        ///
        /// @param displayLanguage[in]        - Language to use for any UI, such as GFN download and install progress dialogs.
        ///                                     Defaults to system language if not defined.
        /// @retval gfnSuccess                - If the SDK was initialized and all SDK features are available.
        /// @retval gfnInitSuccessClientOnly  - If the SDK was initialized, but only client-side functionality is available, such as
        ///                                     calls to gfnStartStream. By definition, gfnIsRunningInCloud is expected to return false
        ///                                     in this scenario.
        /// @retval gfnUnableToAllocateMemory - SDK initialization failed due to lack of available memory, and no SDK functionality is
        ///                                     available.
        /// @retval gfnInitFailure            - SDK initialization failed for any other reason, and no SDK functionality is available.
        GfnRuntimeError gfnInitializeRuntimeSdk(GfnDisplayLanguage displayLanguage);

        ///
        /// @par Description
        /// Releases the SDK, and frees up memory allocated by GFN and disconnects from GFN backend.
        ///
        /// @par Environment
        /// Cloud and Client
        ///
        /// @par Usage
        /// Call during application shutdown or when GFN Runtime API methods are no longer needed.
        void gfnShutdownRuntimeSdk();
        /// @}

        /// @defgroup callbacks Client Callback Registration
        /// Callback registration functions to receieve various messages / requests from the SDK.
        /// Register only after calling gfnInitializeRuntimeSdk().
        /// @{

        ///
        /// @par Description
        /// Register an application function to call when Geforce NOW needs to exit the game.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Register an application function to call when Geforce NOW needs to exit the game.
        ///
        /// @param exitCallback[in] - Function pointer to application code to call when Geforce NOW
        ///                           needs to exit the game.
        /// @param pUserContext[in] - Pointer to user context, which will be passed unmodified to the
        ///                           registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess              - On success, when running in a Geforce NOW environment
        /// @retval gfnDLLNotPresent        - If callback could not be registered
        /// @retval gfnInvalidParameter     - If exitCallback function pointer is NULL.
        /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
        ///                                   was called outside of a cloud execution environment
        GfnRuntimeError gfnRegisterExitCallback(ExitCallbackSig exitCallback, void* pUserContext);

        ///
        /// @par Description
        /// Register an application callback with Geforce NOW to be called when Geforce NOW
        /// needs to pause the game on the user’s behalf. For Multiplayer games, it is
        /// recommended that this is implemented similar to a client disconnect.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Register an application function to call when Geforce NOW needs to pause the game.
        ///
        /// @param pauseCallback[in] - Function pointer to application code to call when
        ///                            Geforce NOW needs to pause the game
        /// @param pUserContext[in]  - Pointer to user context, which will be passed unmodified to the
        ///                            registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess              - On success, when running in a Geforce NOW environment
        /// @retval gfnDLLNotPresent        - If callback could not be registered
        /// @retval gfnInvalidParameter     - If pauseCallback function pointer is NULL.
        /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
        ///                                   was called outside of a cloud execution environment
        GfnRuntimeError gfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* pUserContext);

        ///
        /// @par Description
        /// Register an application callback with Geforce NOW to be called after a successful call to
        /// gfnSetupTitle. Typically, the callback would handle any additional installation steps that
        /// are necessary after Geforce NOW performs its own setup for a given title.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Register a function to call after a successful call to gfnSetupTitle.
        ///
        /// @param gfnRegisterInstallCallback[in] - Function pointer to application code to call after
        ///                                         Geforce NOW successfully performs its own title setup.
        /// @param pUserContext[in]               - Pointer to user context, which will be passed unmodified to the
        ///                                         registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess              - On success, when running in a Geforce NOW environment
        /// @retval gfnDLLNotPresent        - If callback could not be registered
        /// @retval gfnInvalidParameter     - If installCallback function pointer is NULL.
        /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
        ///                                   was called outside of a cloud execution environment
        GfnRuntimeError gfnRegisterInstallCallback(InstallCallbackSig installCallback, void* pUserContext);

        ///
        /// @par Description
        /// Register a callback that gets called on the user's PC when the streaming session state changes
        ///
        /// @par Environment
        /// Client
        ///
        /// @par Usage
        /// Register a function to call when stream status changes on the user's client PC
        ///
        /// @param gfnRegisterStreamStatusCallback[in] - Function pointer to application code to call after
        ///                                              the Geforce NOW streamer changes state
        /// @param pUserContext[in]                    - Pointer to user context, which will be passed unmodified to the
        ///                                              registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess              - On success, when running on the client PC
        /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
        ///                                   was called outside of a client execution environment
        GfnRuntimeError gfnRegisterStreamStatusCallback(StreamStatusCallbackSig streamStatusCallback, void* pUserContext);
        /// @}

        // ============================================================================================
        // Application -> Geforce NOW SDK communication
        // The application should call these methods at the appropriate locations.
        /// @defgroup launcher Launcher Application Methods
        /// The functions is this section are called from a launcher in either the appropriate
        /// cloud or client environment.
        /// @{

        ///
        /// @par Description
        /// Determines if calling application is running in GFN environment or not.
        ///
        /// @par Environment
        /// Cloud and Client
        ///
        /// @par Usage
        /// Use to determine whether to enable / disable any GFN cloud environment specific
        /// application logic, for example, to block any calls to gfnStartStream().
        ///
        /// @retval true  - Application is running on a GFN game seat virtual machine
        ///                 in the Geforce NOW cloud.
        /// @retval false - Application is not running in the Geforce NOW cloud or in a
        ///                 Geforce NOW test environment. This value will also be returned if
        ///                 called before gfnInitializeRuntimeSdk().
        ///
        /// @note
        /// This data is also alternatively accessible in the GFN environment without calling this
        /// API. Please see the "GFN-SDK-ALTERNATIVE-DATA-ACCESS.pdf" document in the "doc" folder
        bool gfnIsRunningInCloud();

        ///
        /// @par Description
        /// Notifies Geforce NOW that an application should be readied for launch. Geforce NOW
        /// will make the build files associated with the application available at the path
        /// returned in the SetupTitleResult struct passed in to the ConfirmTitleSetup callback
        /// provided by the caller. Additionally, Geforce NOW will set all necessary settings to
        /// optimize for the Geforce NOW cloud environment, and download any associated user data,
        /// including save files.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Use to prepare an application for launch on Geforce NOW, and block on the result.
        ///
        /// @param pchPlatformAppId[in] - Identifier of the requested application to setup
        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @retval gfnSetupTitleFailure     - The Geforce NOW SDK failed to set up the title
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnSetupTitle(const char* pchPlatformAppId);

        ///
        /// @par Description
        /// Notifies GFN that an application has exited. GFN will then start the
        /// shutdown process for that application. Note that this is for use by
        /// platform clients only and assumes the application has already completed
        /// execution. To shutdown from within an application itself, gfnShutdownRuntimeSDK is used.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Use to notify GFN that your application has exited.
        ///
        /// @param pchPlatformId[in] - Identifier of the launcher service, e.g. “Steam”
        /// @param pchPlatformAppId[in] - Identifier of the application that has exited.
        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnTitleExited(const char* pchPlatformId, const char* pchPlatformAppId);

        ///
        /// @par Description
        /// Since applications running under Geforce NOW run in the Geforce NOW data centers,
        /// any IP queries made by the Application will return IPs associated
        /// with the data center, not the user’s external client IP, as seen by Internet queries.
        /// This SDK call allows the application to obtain the user's client IP in the IPv4 format
        /// so that developers can make regional business decisions for the user based on it versus
        /// the region of the data center the user is connected to for game streaming sessions.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Call this during application start or from the platform client in
        /// order to get the user’s external client IP address.
        ///
        /// @param ppchClientIp[out] - Output IPv4 in string format. Example: “192.168.0.1”
        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnGetClientIp(const char** ppchClientIp);

        ///
        /// @par Description
        /// Gets user’s client language code  in the form <lang>-<country> using
        /// a standard ISO 639-1 language code and ISO 3166-1 Alpha-2 country code.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Call this during application start or from the platform client in
        /// order to get the user's language and country settings.
        ///
        /// @param ppchLanguageCode[out] - Language code as a string. Example: “en-US”
        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnGetClientLanguageCode(const char** ppchLanguageCode);

        ///
        /// @par Description
        /// Request to obtain a user-specific access token to allow access to
        /// the GFN backend service (IDM endpoint).
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// The access token provided can be used by the application’s backend
        /// servers to validate the user and obtain user data from the GFN
        /// backend service. The GFN backend service provides an OAuth2
        /// interface for validating users and retrieving data. See Account Linking information
        /// in the Overview section for more information.
        ///
        /// @note For security purposes, the access token is provided only to applications
        /// registered at https://devportal.nvgs.nvidia.com, and have a client and an API key
        /// created for them. Contact your NVIDIA partner to have your application registered
        /// and assigned these two keys.
        ///
        /// @param ppchAuthToken[out] - Populated with a user specific GFN access token.
        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @return Otherwise, appropriate error code
        ///
        /// @note
        /// This data is also alternatively accessible in the GFN environment without calling this
        /// API. Please see the "GFN-SDK-ALTERNATIVE-DATA-ACCESS.pdf" document in the "doc" folder
        GfnRuntimeError gfnRequestGfnAccessToken(const char** ppchAuthToken);

        ///
        /// @par Description
        /// Requests GFN client to start a streamed session of an application in an asynchronous fashion
        ///
        /// @par Environment
        /// Client
        ///
        /// @par Usage
        /// Use to start a streaming session.
        ///
        /// @param pStartStreamInput[in] - Pointer to a StartStreamInput structure.
        /// @param cb[in]                - A StartStreamCallbackSig callback to be called with streaming events.
        /// @param context[in]           - User context
        /// @param timeoutMs[in]         - Time after which attempt to start streaming will be aborted.
        void gfnStartStreamAsync(const StartStreamInput * pStartStreamInput, StartStreamCallbackSig cb, void* context, unsigned int timeoutMs);

        ///
        /// @par Description
        /// Requests GFN client to start a streamed session of an application in a synchronous fashion
        ///
        /// @par Environment
        /// Client
        ///
        /// @par Usage
        /// Use to start a streaming session.
        ///
        /// @param pStartStreamInput[in] - Pointer to a StartStreamInput structure.
        /// @param response[in]          - Start streaming response.
        ///
        /// @retval gfnSuccess              - On success
        /// @retval gfnInvalidParameter     - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment - If called in a cloud environment
        /// @retval gfnStreamFailure        - Network failure or other error prevented the stream from starting
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnStartStream(StartStreamInput * pStartStreamInput, StartStreamResponse* response);
        /// Retrieves custom data passed in by the client in the gfnStartStream call.
        ///
        /// @note Function will be implemented in next release.
        ///
        /// @par Environment
        /// Cloud
        ///
        /// @par Usage
        /// Use during cloud session to retrieve custom data
        ///
        /// @param ppchCustomData[out] - Populated with the custom data.
        ///
        /// @retval gfnSuccess              - On success
        /// @retval gfnInvalidParameter     - NULL pointer passed in
        /// @retval gfnCallWrongEnvironment - If called in a client environment
        /// @return Otherwise, appropriate error code
        ///
        GfnRuntimeError gfnGetCustomData(const char** ppchCustomData);
        /// @}

#ifdef __cplusplus
    } // extern "C"
}
#endif


#endif // GFN_RUNTIME_SDK_CAPI_H
