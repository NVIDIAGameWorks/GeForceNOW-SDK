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
/// @mainpage GFN Runtime SDK
///
/// @tableofcontents
///
/// @section introduction Introduction The NVIDIA GFN Runtime SDK provides a set
/// of interfaces to allow game developers and game publishers to interact with
/// parts of the NVIDIA GeForce NOW ecosystem. Integration is provided by
/// various means, from native C interfaces to RESTful Web API calls, depending
/// on the feature of the SDK.
///
/// @image html ecosystem.png
///
/// This document provides details of how to integrate the GFN Runtime SDK
/// features into your application and its developer and deployment processes.
/// For more detailed technical information about the architecture, refer to the
/// architecture section later in this document.
///
/// @section integration C API for Launcher or Game Integration and Single Sign-On (SSO)
/// @subsection architecture Architecture
///
/// @image html architecture.png
///
/// The GFN Runtime SDK provides a static C  library that
/// is linked to the game/application and exposes the various APIs defined in
/// this document. The behavior of those APIs depends on the environment the
/// application is running in; either on a client/user system or in the GeForce
/// NOW (GFN) cloud environment.
///
/// On client systems, this library checks for the presence of the GeForce NOW
/// (GFN) client installation at initialization time. If the client is not
/// present, or the client is out of date, then the library will initiate a
/// download and installation of the latest client on the first API call that
/// requires the client to be present, presenting UI to the user for the
/// installation process.
///
/// If the GFN client is installed on the system, the static library will defer
/// API actions to components in the installation.
///
/// This design allows the static library to stay as thin as possible in the
/// application, and provides backward and forward compatibility to new GFN
/// client packages.
///
/// On GFN game systems, many of the APIs are no-ops as they apply only to
/// client/end-user systems. In those cases, API calls will return a
/// well-defined error code to denote the call was not applicable to the
/// environment.
///
/// @subsection authentication Authentication and Federation NVIDIA provides a backend IDM
/// Service that application developers can use to validate users and obtain
/// user information from. This provides a seamless flow for users to bypass
/// multiple login steps in streaming a game through GFN.
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
/// @subsection api GFN Runtime API Overview
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
/// @subsection general_section General / Common methods
/// @ref general
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnInitializeGfnRuntimeSdk
/// @copydoc gfnInitializeGfnRuntimeSdk
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref gfnShutdownGfnRuntimeSdk
/// @copydoc gfnShutdownGfnRuntimeSdk
///
/// @subsection launcher_section Launcher Application-specific Methods
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
/// C        | @ref gfnRequestKeyboardOverlayOpen
/// C        | @ref gfnRequestKeyboardOverlayClose
///
/// @copydoc gfnRequestKeyboardOverlay


#ifndef GFN_RUNTIME_SDK_CAPI_H
#define GFN_RUNTIME_SDK_CAPI_H

#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
typedef char bool;
#define false 0
#define true 1
#endif

#define GFN_RUNTIME_SDK_VERSION 1.2

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
            gfnDLLNotPresent =           -1,
            gfnComNotEstablished =       -2, ///< SDK communication channels opened, but no Geforce Now components found.
            gfnComError =                -3, ///< Geforce Now SDK internal component communication error.
            gfnErrorCallingDLLFunction = -4, ///< Geforce Now SDK components were reachable, but could not serve the request.
            gfnIncompatibleVersion =     -5,
            gfnUnableToAllocateMemory =  -6,
            gfnInvalidParameter =        -7,
            gfnInternalError =           -8, ///< Generic Geforce Now SDK internal error.
            gfnUnsupportedAPICall =      -9,
            gfnInvalidToken =            -10,
            gfnTimedOut =                -11,
            gfnSetupTitleFailure =       -12,
			gfnClientDownloadFailed =    -13 ///< Failed to download the Geforce Now client.
        } GfnRuntimeError;

        /* @brief Returned by callbacks the application registers with the Geforce Now Runtime SDK, or passes
         *  in to asynchronous SDK calls. By default, the SDK will return crCallbackNotRegistered for you
         *  if your application has not registered a particular callback. */
        typedef enum GfnApplicationCallbackResult
        {
            crCallbackSuccess =  0, ///< Return to indicate that a callback has performed the requested operation
            crCallbackFailure = -1  ///< Return to indicate that a callback did not perform the requested operation
        } GfnApplicationCallbackResult;

        /// @brief Parameter to RequestKeyboardOverlay method. Specifies where to display text input element on screen
        typedef enum GFNScreenPosition
        {
            gspBottom = 0,
            gspTop,
            gspLeft,
            gspRight,
            gspCenter,
            gspTopLeft,
            gspTopRight,
            gspBottomLeft,
            gspBottomRight
        } GFNScreenPosition;

        /// @brief Output response when streaming has started
        typedef struct StartStreamResponse
        {
            bool downloaded; ///< True if Geforce Now client components were downloaded from the release site
        } StartStreamResponse;

        /// @brief Input to the function registered via gfnRegisterInstallCallback (if any).
        typedef struct TitleInstallationInformation
        {
            const char* pchPlatformAppId; ///< Platform application ID passed into the API.
            const char* pchBuildPath;     ///< The path at which game build files can be found.
            const char* pchMetadataPath;  ///< Optionally contains the path at which game metadata can be found, else NULL.
        } TitleInstallationInformation;

        // ============================================================================================
        // Callback signatures
        // ============================================================================================

        // Callbacks that client code should implement and register via the appropriate callback
        // registration function (listed as part of the C API below). The return value will typically be
        // GfnApplicationCallbackResult::crCallbackSuccess or GfnApplicationCallbackResult::crCallbackFailure.
        typedef GfnApplicationCallbackResult(*ExitCallbackSig)(void* pUserContext);  // Register via gfnRegisterExitCallback
        typedef GfnApplicationCallbackResult(*PauseCallbackSig)(void* pUserContext); // Register via gfnRegisterPauseCallback
        typedef GfnApplicationCallbackResult(*InstallCallbackSig)(const TitleInstallationInformation* pInfo, void* pUserContext); // Register via gfnRegisterInstallCallback

        // Callbacks into client code passed as arguments to asynchronous API calls.
        typedef void(*StartStreamCallbackSig)(GfnRuntimeError, StartStreamResponse*, void* context);

        // ============================================================================================
        // C API
        // ============================================================================================

        // ============================================================================================
        // SDK Initialization / Shutdown

        /// @defgroup general General / Common Methods
        /// @{

        /// @brief Initialize the GFN Runtime
        ///
        /// @par Description
        /// Should be called at application startup and prior to any GFN Runtime
        /// API methods. When running outside of a GFN environment (a game seat
        /// virtual machine or development environment) it is expected for this
        /// method to return a result other than success. In this case all GFN
        /// methods become no-ops and have no performance impact on your
        /// application.
        ///
        /// @par Usage
        /// Call as soon as possible during application startup.
        ///
        /// @retval gfnSuccess - On success when running in a GFN environment
        /// @retval gfnGfnDLLNotPresent - if running outside a GFN environment (No GFN.dll present)
        /// @retval gfnGfnComNotEstablished - if running outside a GFN environment (No GFN host or test app running)
        /// @retval gfnIncompatibleVersion - Linked GFN Runtime SDK library not compatible
        GfnRuntimeError gfnInitializeGfnRuntimeSdk();

        /// @brief Free the GFN Runtime
        ///
        /// @par Description
        /// Releases the SDK, and frees up memory allocated by GFN and
        /// disconnects from GFN backend.
        ///
        /// @par Usage
        /// Call during application shutdown or when GFN Runtime API
        /// methods are no longer needed.
        void gfnShutdownGfnRuntimeSdk();
        /// @}

        // ============================================================================================
        // Callback registration functions (Geforce Now SDK -> Application communication)
        // Register callbacks here in order to receieve various messages / requests from the SDK.
        // Register only after calling gfnInitializeGfnRuntimeSdk().
        /// @defgroup callbacks Client Callback Registration
        /// @{

        /// @brief Optionally, register a function to call when Geforce Now needs to exit the game
        ///
        /// @par Description
        /// Register an application function to call when Geforce Now needs to exit the game.
        ///
        /// @par Usage
        /// Register an application function to call when Geforce Now needs to exit the game.
        ///
        /// @param exitCallback[in] - Function pointer to application code to call when Geforce Now
        ///                           needs to exit the game
        /// @param pUserContext[in] - Pointer to user context, which will be passed unmodified to the
        ///                           registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess       - On success, when running in a Geforce Now environment
        /// @retval gfnDLLNotPresent - If callback could not be registered
        GfnRuntimeError gfnRegisterExitCallback(ExitCallbackSig exitCallback, void* pUserContext);

        /// @brief Optionally, register a function to call when Geforce Now needs to pause the game
        ///
        /// @par Description
        /// Register an application callback with Geforce Now to be called when Geforce Now
        /// needs to pause the game on the user’s behalf. For Multiplayer games, it is
        /// recommended that this is implemented similar to a client disconnect.
        ///
        /// @par Usage
        /// Register an application function to call when Geforce Now needs to pause the game.
        ///
        /// @param pauseCallback[in] - Function pointer to application code to call when
        ///                            Geforce Now needs to pause the game
        /// @param pUserContext[in]  - Pointer to user context, which will be passed unmodified to the
        ///                            registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess       - On success, when running in a Geforce Now environment
        /// @retval gfnDLLNotPresent - If callback could not be registered
        GfnRuntimeError gfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* pUserContext);

        /// @brief Optionally, register a function to call after a successful call to gfnSetupTitle
        ///
        /// @par Description
        /// Register an application callback with Geforce Now to be called after a successful call to
        /// gfnSetupTitle. Typically, the callback would handle any additional installation steps that
        /// are necessary after Geforce Now performs its own setup for a given title.
        ///
        /// @par Usage
        /// Register a function to call after a successful call to gfnSetupTitle.
        ///
        /// @param gfnRegisterInstallCallback[in] - Function pointer to application code to call after
        ///                                         Geforce Now successfully performs its own title setup.
        /// @param pUserContext[in]               - Pointer to user context, which will be passed unmodified to the
        ///                                         registered callback. Can be NULL.
        ///
        /// @retval gfnSuccess       - On success, when running in a GFN environment
        /// @retval gfnDLLNotPresent - If callback could not be registered
        GfnRuntimeError gfnRegisterInstallCallback(InstallCallbackSig installCallback, void* pUserContext);
        /// @}

        // ============================================================================================
        // Application -> Geforce Now SDK communication
        // The application should call these methods at the appropriate locations.
        /// @defgroup launcher Launcher Application Methods
        /// @{

        /// @brief Determines if application is running in GFN environment
        ///
        /// @par Description
        /// Determines if calling application is running in GFN environment or not.
        ///
        /// @par Usage
        /// Use to enable any GFN environment specific application logic, for
        /// example, to block any calls to @ref gfnStartStream().
        ///
        /// @retval true  - Application is running on a game seat virtual machine
        ///                 in the Geforce Now cloud, or in a Geforce Now test environment.
        /// @retval false - Application is not running in the Geforce Now cloud or in a
        ///                 Geforce Now test environment.
        bool gfnIsRunningInCloud();

        /// @brief Notifies GFN to ready an application for launch. This is a blocking call.
        ///
        /// @par Description
        /// Notifies Geforce Now that an application should be readied for launch. Geforce Now
        /// will make the build files associated with the application available at the path
        /// returned in the SetupTitleResult struct passed in to the ConfirmTitleSetup callback
        /// provided by the caller. Additionally, Geforce Now will set all necessary settings to
        /// optimize for the Geforce Now cloud environment, and download any associated user data,
        /// including save files.
        ///
        /// @par Usage
        /// Use to prepare an application for launch on Geforce Now, and block on the result.
        ///
        /// @param pchPlatformAppId[in] - Identifier of the requested application to setup
        ///
        /// @retval gfnSuccess           - On success
        /// @retval gfnSetupTitleFailure - The Geforce Now SDK failed to set up the title
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnSetupTitle(const char* pchPlatformAppId);

        /// @brief Notifies GFN that an application has exited
        ///
        /// @par Description
        /// Notifies GFN that an application has exited. GFN will then start the
        /// shutdown process for that application. Note that this is for use by
        /// platform clients only and assumes the application has already completed
        /// execution. To shutdown from within an application itself,
        /// ShutdownGFNRuntimeSDK is used.
        ///
        /// @par Usage
        /// Use to notify GFN that your application has exited.
        ///
        /// @param pchPlatformId[in] - Identifier of the launcher service, e.g. “Steam”
        /// @param pchPlatformAppId[in] - Identifier of the application that has exited.
        ///
        /// @retval gfnSuccess  - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnTitleExited(char* pchPlatformId, const char* pchPlatformAppId);

        /// @brief Gets user’s client IPv4 address
        ///
        /// @par Description
        /// Since Applications running under Geforce Now run in the Geforce Now data centers,
        /// any IP queries made by the Application will return IPs associated
        /// with the data center, not the user’s client IP. This SDK call allows the
        /// Application to obtain the client IP in the IPv4 format so that developers
        /// can make regional business decisions.
        ///
        /// @par Usage
        /// Call this during application start or from the platform client in
        /// order to get the user’s actual client IP address.
        ///
        /// @param ppchClientIp[out] - Output IPv4 in string format. Example: “192.168.0.1”
        ///
        /// @retval gfnSuccess  - On success
        /// @return Otherwise, appropriate error code
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnGetClientIp(const char** ppchClientIp);

        /// @brief Gets user’s client language code
        ///
        /// @par Description
        /// Gets user’s client language code  in the form <lang>-<country> using
        /// a standard ISO 639-1 language code and ISO 3166-1 Alpha-2 country code.
        ///
        /// @par Usage
        /// Call this during application start or from the platform client in
        /// order to get the user’s language and country settings
        ///
        /// @param ppchLanguageCode[out] - Language code as a string. Example: “en-US”
        ///
        /// @retval gfnSuccess - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnGetClientLanguageCode(const char** ppchLanguageCode);

        /// @brief Get user's NVIDIA authentication token
        ///
        /// @par Description
        /// Request to obtain a user specific access token to allow access to
        /// the GFN backend service (IDM endpoint).
        ///
        /// @par Usage
        /// The access token provided can be used by the application’s backend
        /// servers to validate the user and obtain user data from the GFN
        /// backend service. The GFN backend service provides an OAuth2
        /// interface for validating users and retrieving data. See Account
        /// Federation section for more information.
        ///
        /// @param ppchAuthToken[out] - Populated with a user specific GFN access token.
        /// @param pchClientId[in]    - The Jarvis identifier for your application.
        ///
        /// @retval gfnSuccess - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnRequestGfnAccessToken(const char* pchClientId, const char** ppchAuthToken);

        /// @brief Start streaming an application asynchronously
        ///
        /// @par Description
        /// Requests GFN client to start a streamed session of an application
        ///
        /// @par Usage
        /// Use to start a streaming session.
        ///
        /// @param pchServiceId[in] - Populated with a provider specific unique identifier.
        /// @param pchAppId[in]     - Populated with a game specific unique identifier.
        /// @param pchAuthToken[in] - Delegate token from NVIDIA IDM.
        /// @param cb[in]           - Callback to be called with streaming events.
        /// @param context[in]      - User context
        /// @param timeoutMs[in]    - Time after which attempt to start streaming will be aborted.
        void gfnStartStreamAsync(const char* pchServiceId, const char* pchAppId, const char* pchAuthToken, StartStreamCallbackSig cb, void* context, int timeoutMs);

        /// @brief Start streaming an application
        ///
        /// @par Description
        /// Requests GFN client to start a streamed session of an application
        ///
        /// @par Usage
        /// Use to start a streaming session.
        ///
        /// @param pchServiceId[in] - Populated with a provider specific unique identifier.
        /// @param pchAppId[in]     - Populated with a game specific unique identifier.
        /// @param pchAuthToken[in] - Delegate token from NVIDIA IDM.
        /// @param response[in]     - Start streaming response.
        /// @param timeoutMs[in]    - Time after which attempt to start streaming will be aborted.
        ///
        /// @retval gfnSuccess - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnStartStream(const char* pchServiceId, const char* pchAppId, const char* pchAuthToken, StartStreamResponse* response, int timeoutMs);

        /// @brief Called from application when text input is needed to continue
        ///
        /// @par Description
        /// Open the onscreen keyboard in the specified screen position.
        ///
        /// @par Usage
        /// Use to allow keyboard style input when required, if the control scheme is known
        /// to use input other than a standard keyboard (such as a controller).
        ///
        /// @param gspPosition[in] - The screen position at which the onscreen keyboard should display
        ///
        /// @retval gfnSuccess - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnRequestKeyboardOverlayOpen(GFNScreenPosition gspPosition);

        /// @brief Called when any required text input has been processed and the onscreen keyboard can be closed
        ///
        /// @par Description
        /// Closes the onscreen keyboard if it is open.
        ///
        /// @par Usage
        /// Once any required keyboard-style input has been received and / or processed, close the onscreen keyboard.
        ///
        /// @retval gfnSuccess - On success
        /// @return Otherwise, appropriate error code
        GfnRuntimeError gfnRequestKeyboardOverlayClose();
        /// @}

#ifdef __cplusplus
    } // extern "C"
}
#endif


#endif // GFN_RUNTIME_SDK_CAPI_H
