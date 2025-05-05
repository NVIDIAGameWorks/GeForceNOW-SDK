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

//
// ===============================================================================================
//
// C-based wrapper API declarations for GFN Runtime SDK APIs
//
// ===============================================================================================
/**
* @file GfnRuntimeSdk_Wrapper.h
*
* Wrapper API definitions
*/
///
/// @page wrapper_apis Native Runtime API Export Wrapper Reference
///
/// @section wrapper_introduction Introduction
/// The NVIDIA GFN Runtime SDK provides a set of APIs as export functions from a dynamic
/// library. To ease integration and use of these APIs, the GFN Runtime SDK provides
/// this optional set of pure-C functions that manage the lifetime of the library as well as
/// wrap the export function calls. This allows integrators to avoid having to work with
/// export functions directly if they so choose.
///
/// Most of the functions defined in this header file have the same definitions and calling
/// characteristics as the export function they wrap. As such, they will return the same
/// data types as well as error codes. As such, the documentation for each is purposefully
/// pared down to avoid duplication. There are a few exceptions, and those functions will have
/// more extensive documentation that calls out the differences.
///
/// @section wrapper_api_reference API Wrapper Reference
/// @subsection wrapper_section Wrapper Methods
/// @ref wrapper
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnInitializeSdk
///
/// @copydoc GfnInitializeSdk
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnShutdownSdk
///
/// @copydoc GfnShutdownSdk
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnIsRunningInCloud
///
/// @copydoc GfnIsRunningInCloud
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnIsRunningInCloudSecure
///
/// @copydoc GfnIsRunningInCloudSecure
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnFree
///
/// @copydoc GfnFree
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetClientIpV4
///
/// @copydoc GfnGetClientIpV4
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetClientIpV4
///
/// @copydoc GfnGetClientIpV4
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetClientLanguageCode
///
/// @copydoc GfnGetClientLanguageCode
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetPartnerData
///
/// @copydoc GfnGetPartnerData
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnIsTitleAvailable
///
/// @copydoc GfnIsTitleAvailable
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetTitlesAvailable
///
/// @copydoc GfnGetTitlesAvailable
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetClientInfo
///
/// @copydoc GfnGetClientInfo
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnGetSessionInfo
///
/// @copydoc GfnGetSessionInfo
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterStreamStatusCallback
///
/// @copydoc GfnRegisterStreamStatusCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnStartStream
///
/// @copydoc GfnStartStream
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnStartStreamAsync
///
/// @copydoc GfnStartStreamAsync
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnSetupTitle
///
/// @copydoc GfnSetupTitle
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnTitleExited
///
/// @copydoc GfnTitleExited
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterExitCallback
///
/// @copydoc GfnRegisterExitCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterPauseCallback
///
/// @copydoc GfnRegisterPauseCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterInstallCallback
///
/// @copydoc GfnRegisterInstallCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterSaveCallback
///
/// @copydoc GfnRegisterSaveCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterSessionInitCallback
///
/// @copydoc GfnRegisterSessionInitCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnAppReady
///
/// @copydoc GfnAppReady
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnSetActionZone
///
/// @copydoc GfnSetActionZone
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnSendMessage
///
/// @copydoc GfnSendMessage
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterClientInfoCallback
///
/// @copydoc GfnRegisterClientInfoCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterNetworkStatusCallback
///
/// @copydoc GfnRegisterNetworkStatusCallback
///
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnRegisterMessageCallback
///
/// @copydoc GfnRegisterMessageCallback
///
/// Language | API
/// -------- | -------------------------------------
/// C        | @ref GfnOpenURLOnClient
///
/// @copydoc GfnOpenURLOnClient

#include "GfnRuntimeSdk_CAPI.h"

#include <stddef.h>

#ifdef _WIN32
    #define CHAR_TYPE wchar_t
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    /// @brief Global handle to loaded GFN SDK Library module
    extern HMODULE g_gfnSdkModule;
#elif __linux__
    #define CHAR_TYPE char
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    /// @defgroup wrapper API Wrapper Methods
    /// @{

    /// @par Description
    /// Loads the GFN SDK dynamic library and calls @ref gfnInitializeRuntimeSdk. By default, the function
    /// expects the library to be in the same folder as the loading process's executable. For
    /// security reasons, the dynamic library is loaded by fully-quanitified path. If the GFN SDK
    /// library is packaged in another folder, you will need to locally modify the function to
    /// reference that location.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call as soon as possible during application startup.
    ///
    /// @param language                   - Language to use for any UI, such as GFN download and install progress dialogs.
    ///                                     Defaults to system language if not defined.
    /// @retval gfnSuccess                - If the SDK was initialized and all SDK features are available.
    /// @retval gfnInitSuccessClientOnly  - If the SDK was initialized, but only client-side functionality is available, such as
    ///                                     calls to gfnStartStream. By definition, gfnIsRunningInCloud is expected to return false
    ///                                     in this scenario.
    /// @retval gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    /// @retval gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnInitializeSdk(GfnDisplayLanguage language);

    /// @brief Backward compatibility alias for GfnInitializeSdkFromPathA
    #define GfnInitializeSdkFromPath GfnInitializeSdkFromPathA

    /// @par Description
    /// Loads the GFN SDK dynamic library from the given path and calls @ref gfnInitializeRuntimeSdk. (UTF-8 / ASCII version)
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call as soon as possible during application startup.
    ///
    /// @param language                   - Language to use for any UI, such as GFN download and install progress dialogs.
    ///                                     Defaults to system language if not defined.
    /// @param sdkLibraryPath             - Fully-quantified path to GFN SDK library file including the library filename,
    ///                                     note that for security reasons, library cannot be renamed to any other string.
    /// @retval gfnSuccess                - If the SDK was initialized and all SDK features are available.
    /// @retval gfnInitSuccessClientOnly  - If the SDK was initialized, but only client-side functionality is available, such as
    ///                                     calls to gfnStartStream. By definition, gfnIsRunningInCloud is expected to return false
    ///                                     in this scenario.
    /// @retval gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    /// @retval gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnInitializeSdkFromPathA(GfnDisplayLanguage language, const char* sdkLibraryPath);

    /// @par Description
    /// Loads the GFN SDK dynamic library from the given path and calls @ref gfnInitializeRuntimeSdk. (Wide char version)
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call as soon as possible during application startup.
    ///
    /// @param language                   - Language to use for any UI, such as GFN download and install progress dialogs.
    ///                                     Defaults to system language if not defined.
    /// @param wSdkLibraryPath            - Wide Char string with fully-quantified path to GFN SDK library file including the library
    ///                                     filename, note that for security reasons, library cannot be renamed to any other string.
    /// @retval gfnSuccess                - If the SDK was initialized and all SDK features are available.
    /// @retval gfnInitSuccessClientOnly  - If the SDK was initialized, but only client-side functionality is available, such as
    ///                                     calls to gfnStartStream. By definition, gfnIsRunningInCloud is expected to return false
    ///                                     in this scenario.
    /// @retval gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    /// @retval gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnInitializeSdkFromPathW(GfnDisplayLanguage language, const wchar_t* wSdkLibraryPath);

    ///
    /// @par Description
    /// Calls @ref gfnShutdownRuntimeSdk to releases the SDK and resources and disconnects from GFN
    /// backend and then unloads unloads the GFN SDK Library.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call during application shutdown or when GFN Runtime API methods are no longer needed.
    /// @retval gfnSuccess                - If the SDK was initialized and all SDK features are available
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnShutdownSdk(void);

    ///
    /// @par Description
    /// Calls @ref gfnIsRunningInCloud to determine if calling application is running in GFN environment,
    /// and without requiring process registration.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to quickly determine whether to enable / disable any low-value GFN cloud environment
    /// specific application logic, for example, to block any calls to @ref GfnStartStream() to
    /// avoid an error, or to know if @ref GfnGetTitlesAvailable can be called without an error.
    ///
    /// @warning
    /// This API is meant to fill the need to quickly determine if the call looks to be in the
    /// GFN cloud environment. It purposefully trades off resource-intensive checks for fast response.
    /// Do not tie any logic or features of value to this API as the call stack could be tampered with.
    /// For that purpose, use @ref GfnIsRunningInCloudSecure.
    ///
    /// @param runningInCloud             - Pointer to a boolean that receives true if running in GeForce NOW
    ///                                     cloud or false if not in the GeForce NOW cloud
    /// @retval gfnSuccess                - If the query was successful
    /// @retval gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    /// @retval gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    ///
    /// @note
    /// This function's definition differs from the export that it wraps, as it returns a
    /// GfnRuntimeError instead of bool.
    GfnRuntimeError GfnIsRunningInCloud(bool* runningInCloud);

    ///
    /// @par Description
    /// Calls @ref gfnIsRunningInCloudSecure to determine if calling application is running in GFN environment,
    /// and what level of security assurance that the result is valid.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Call from an NVIDIA-approved process to securely determine whether running in GFN cloud, and use the
    /// GfnIsRunningInCloudAssurance value to decide the risk to enable any application specific logic
    /// for that environment.
    ///
    /// @warning
    /// This API must be called from a process that has been registered with NVIDIA, or it will return an error.
    /// Refer to the Cloud Check API Guide on how to get your application registered. To prevent
    /// man-in-the-middle (MiM) attacks, you must also securely load the SDK library, checking the integrity
    /// of the digital signature on the binary. Make sure to use the value returned from GfnIsRunningInCloudAssurance
    /// to decide if the check was certain enough without tampering to enable the logic or feature associated
    /// with the API call.
    ///
    /// @param assurance                  - Likelihood and level of security assurance defined via @ref GfnIsRunningInCloudAssurance that API is running in GFN cloud environment
    ///
    /// @retval gfnSuccess                - If the query was successful.
    /// @retval gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    /// @retval gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnIsRunningInCloudSecure(GfnIsRunningInCloudAssurance* assurance);

    ///
    /// @par Description
    /// Determines if calling application is running in GFN environment or not. It also provides a 
    /// cryptographically signed response if requested, which can be verified either locally or via 3rd party backend.
    /// Refer to the Cloud Check API Guide on how to validate attestation data in response.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// This API can be used from any execution context - privileged or not. 
    ///
    /// @param challenge                 - Optional input parameter, that can be used to pass in nonce data.
    ///                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    /// 
    /// @param response                  - Optional output parameter, that receives the signed attestation response from the API.
    /// 
    /// @param isCloudEnvironment        - Optional output parameter, that receives true value if the caller is in the GFN environment.
    ///
    /// @retval gfnSuccess               - On success indicates cloud check was performed successfully.
    /// @retval gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    /// @retval gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    ///                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    /// @retval gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    /// @retval gfnThrottled             - API call was throttled for exceeding limit
    GfnRuntimeError GfnCloudCheck(const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, bool* isCloudEnvironment);

    ///
    /// @par Description
    /// Determines which GFN environment, if any, is available. This is not useful for most developers.
    /// Only use if you've been instructed to by NVIDIA support.
    /// 
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// This API can be used from any execution context - privileged or not. 
    ///
    /// @param requested_cloud_type      - The desired type of the cloud (CC_CLOUD_TYPE_TRUSTED, CC_CLOUD_TYPE_OPEN, or CC_CLOUD_TYPE_ANY) that the caller wants to check for.
    /// @param challenge                 - Optional input parameter, that can be used to pass in nonce data.
    ///                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    /// 
    /// @param response                  - Optional output parameter, that receives the signed attestation response from the API.
    /// 
    /// @param detected_cloud_type         - An optional parameter to receive the actual type of the environment detected at runtime (CC_CLOUD_TYPE_xxx).
    ///                                    This parameter is only useful when multiple cloud types were provided on input in the requested_cloud_type parameter.
    ///                                    A null pointer can be passed if the caller is not interested in receiving the actual cloud type.
    ///
    /// @retval gfnSuccess               - On success indicates cloud check was performed successfully.
    /// @retval gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    /// @retval gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    ///                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    /// @retval gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    /// @retval gfnUnsupportedAPICall    - When called from a Linux environment.
    /// @retval gfnThrottled             - API call was throttled for exceeding limit
    GfnRuntimeError GfnGetCloudType(
        const GfnCloudType requested_cloud_type,
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response,
        GfnCloudType* detected_cloud_type);

    ///
    /// @par Description
    /// Determines if calling application is running in the "trusted" cloud environment.
    ///
    /// This call is functionally equivalent to GfnGetCloudType(CC_CLOUD_TYPE_TRUSTED, &challenge, &response, &detectedCloudEnvironment);
    /// 
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// This API can be used from any execution context - privileged or not. 
    ///
    /// @param challenge                 - Optional input parameter, that can be used to pass in nonce data.
    ///                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    /// 
    /// @param response                  - Optional output parameter, that receives the signed attestation response from the API.
    ///
    /// @retval gfnSuccess               - On success indicates cloud check was performed successfully.
    /// @retval gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    /// @retval gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    ///                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    /// @retval gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    /// @retval gfnThrottled             - API call was throttled for exceeding limit
    /// @retval gfnCallWrongEnvironment  - The call succeeded, but the application is not executing in the trusted cloud environment.
    GfnRuntimeError GfnGetTrustedCloud(
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response);

    /// @par Description
    /// Determines if calling application is running in the "open" cloud environment.
    /// This is not useful for most developers.
    /// Only use if you've been instructed to by NVIDIA support.
    ///
    /// This call is functionally equivalent to GfnGetCloudType(CC_CLOUD_TYPE_OPEN, &challenge, &response, &actualCloudEnvironment);
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// This API can be used from any execution context - privileged or not. 
    ///
    /// @param challenge                 - Optional input parameter, that can be used to pass in nonce data.
    ///                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    /// 
    /// @param response                  - Optional output parameter, that receives the signed attestation response from the API.
    ///
    /// @retval gfnSuccess               - On success indicates cloud check was performed successfully.
    /// @retval gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    /// @retval gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    ///                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    /// @retval gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    /// @retval gfnThrottled             - API call was throttled for exceeding limit
    /// @retval gfnCallWrongEnvironment  - The call succeeded, but the application is not executing in the open cloud environment.
    GfnRuntimeError GfnGetOpenCloud(
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response);
    ///
    /// @par Description
    /// Calls @ref gfnGetClientIp to get user client's IP address.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call this during application start or from the platform client in
    /// order to get the user's external client IP address.
    ///
    /// @param clientIp - Output IPv4 in string format. Example: "192.168.0.1". Call @ref GfnFree to free the memory.
    ///
    /// @retval gfnSuccess               - Successfully retrieved client IP
    /// @retval gfnNoData                - No IP data found
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    /// @note
    /// To avoid leaking memory, call @ref GfnFree once done with the data.
    GfnRuntimeError GfnGetClientIpV4(const char** clientIp);

    ///
    /// @par Description
    /// Calls @ref gfnGetClientLanguageCode to gets user's client language code in the form
    /// "<lang>-<country>" using a standard ISO 639-1 language code and ISO 3166-1 Alpha-2 country code.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call this during application start or from the platform client in
    /// order to get the user's language and country settings.
    ///
    /// @param languageCode - Language code as a string. Example: "en-US". Call @ref GfnFree to free the memory.
    ///
    /// @retval gfnSuccess               - Successfully retrieved language code
    /// @retval gfnNoData                - No language code found
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    /// @note
    /// To avoid leaking memory, call @ref GfnFree once done with the data.
    GfnRuntimeError GfnGetClientLanguageCode(const char** languageCode);

    ///
    /// @par Description
    /// Gets user's client country code using ISO 3166-1 Alpha-2 country code.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call this during application start or from the platform client in order to get
    /// the user's country code.
    ///
    /// @param[out] countryCode          - Country code as a 2 character string. Example: “US”
    /// @param length                    - Length of pchCountryCode character array
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetClientCountryCode(char* countryCode, unsigned int length);

    ///
    /// @par Description
    /// Gets user's client data.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Call this during application start or from the platform client in order to get
    /// the data on the user's client.
    ///
    /// @param[out] clientInfo           - A structure to hold clientInfo data
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    /// @note
    /// The client data returned by this function is best effort, as not all client devices will report
    /// all information. For example, certain browser and TV clients will not report client resolution or
    /// will incorrect resolution in certain situations. In the cases the data is not reported, the data
    /// returned will be zeroes. Likewise for safezone information, as most clients, such as Windows PC,
    /// do not have screen areas unsafe for rendering or input. For more information about safe zones,
    /// see the  the Mobile Integration Guide in the /doc folder.
    GfnRuntimeError GfnGetClientInfo(GfnClientInfo* clientInfo);

        ///
        /// @par Description
        /// Gets various information about the current streaming session
        ///
        /// @par Environment
        /// Cloud
        /// 
        /// @par Platform
        /// Windows, Linux
        ///
        /// @par Usage
        /// Call this from a streaming session to find out more information about the session, such
        /// as session time remaining, or if RTX is enabled for the current session.
        ///
        /// @param sessionInfo               - Pointer to a GfnSessionInfo struct.

        ///
        /// @retval gfnSuccess               - On success
        /// @retval gfnInvalidParameter      - NULL pointer passed in or buffer length is too small
        /// @retval gfnCallWrongEnvironment  - If called in a client environment
        /// @return Otherwise, appropriate error code
        /// @note
        /// If the application has called @ref gfnRegisterSessionInitCallback to be notified when a
        /// user connects, then this API should be called after that callback is triggered.
        /// Certain data, such as session time limit or RTX support, can only be defined when a user
        /// connects as the values depend on the user type. Calling before that point can result in
        /// obtaining incorrect data.
        GfnRuntimeError GfnGetSessionInfo(GfnSessionInfo* sessionInfo);

    ///
    /// @par Description
    /// Calls @ref GfnGetPartnerData to retrieves non-secure partner data that is either a) passed by the client in the gfnStartStream call
    /// or b) sent using Deep Link parameter.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use during cloud session to retrieve partner data
    ///
    /// @param partnerData              - Populated with the partner data in string form if found
    ///                                   Call @ref GfnFree to free the memory
    ///
    /// @retval gfnSuccess              - Partner data successfully retrieved from session data
    /// @retval gfnNoData               - No partner data found in session data
    /// @retval gfnInvalidParameter     - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GFN SDK Library
    /// @note
    /// To avoid leaking memory, call @ref GfnFree once done with the data.
    GfnRuntimeError GfnGetPartnerData(const char** partnerData);

    ///
    /// @par Description
    /// Calls @ref GfnGetPartnerSecureData to retrieves secure partner data that is either a) passed by the client in the gfnStartStream call
    /// or b) sent in response to Deep Link nonce validation.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use during cloud session to retrieve secure partner data
    ///
    /// @param partnerSecureData         - Populated with the secure partner data in string for if found
    ///                                    Call @ref GfnFree to free the memory
    ///
    /// @retval gfnSuccess               - Secure partner data successfully retrieved from session data
    /// @retval gfnNoData                - No secure partner data found in session data
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    /// @note
    /// To avoid leaking memory, call @ref GfnFree once done with the data.
    GfnRuntimeError GfnGetPartnerSecureData(const char** partnerSecureData);

    ///
    /// @par Description
    /// Calls @ref gfnIsTitleAvailable to determines if a specific title is available to launch
    /// in current streaming session.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to determine if a title is available to be launched from the active GFN cloud instance,
    /// for example, to show a "Play" button in a platform launcher's UI.
    ///
    /// @param platformAppId             - Identifier of the requested title to check
    /// @param isAvailable               - Pointer to a boolean that receives true if the title is
    ///                                    available, or or false if not available.
    ///
    /// @retval gfnSuccess               - If the query was successful.
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    ///
    /// @note
    /// This function's definition differs from the export that it wraps, as it returns a
    /// GfnRuntimeError instead of bool.
    GfnRuntimeError GfnIsTitleAvailable(const char* platformAppId, bool* isAvailable);

    ///
    /// @par Description
    /// Calls @ref GfnGetTitlesAvailable to retrieves all titles that can be launched in the
    /// current game streaming session.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to retrieve a list of all titles available to launch in the current streaming session,
    /// for example, to add "Play" buttons to all titles instead of calling gfnIsTitleAvailable on
    /// each title.
    ///
    /// @param platformAppIds            - Comma-delimited list of platform identifiers. Memory is
    ///                                    allocated for the list. Call @ref GfnFree to free the memory.
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    ///
    /// @note
    /// To avoid leaking memory, call @ref GfnFree once done with the title list.
    GfnRuntimeError GfnGetTitlesAvailable(const char** platformAppIds);

    ///
    /// @par Description
    /// Calls @ref gfnFree to free memory allocated by @ref GfnGetTitlesAvailable
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to release memory after a call to a memory-allocated function and you are finished with the data.
    /// Should only be called if the memory is populated with valid data. Calling @ref GfnFree with invalid
    /// pointer or data will result in an memory exception being thrown.
    ///
    /// @param data                      - Pointer to allocated string memory
    ///
    /// @retval gfnSuccess               - Memory successfully released
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    GfnRuntimeError GfnFree(const char** data);


    ///
    /// @par Description
    /// Calls @ref gfnRegisterStreamStatusCallback to register a callback that gets called on the
    /// user's PC when the streaming session state changes.
    ///
    /// @par Environment
    /// Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Register a function to call when stream status changes on the user's client PC
    ///
    /// @param streamStatusCallback      - Function pointer to application code to call after
    ///                                    the GeForce NOW streamer changes state
    /// @param userContext               - Pointer to user context, which will be passed unmodified to the
    ///                                    registered callback. Can be NULL
    ///
    /// @retval gfnSuccess               - On success, when running on the client PC
    /// @retval gfnCallWrongEnvironment  - If callback could not be registered since this function
    /// @retval gfnClientLibraryNotFound - GFN SDK client-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnRegisterStreamStatusCallback(StreamStatusCallbackSig streamStatusCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnStartStream to request GFN client to start a streaming session of an application
    /// in a synchronous (blocking) fashion.
    ///
    /// @par Environment
    /// Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Use to start a streaming session.
    ///
    /// @param startStreamInput         - Pointer to a StartStreamInput structure.
    /// @param response                 - Start streaming response.
    ///
    /// @retval gfnSuccess              - On success
    /// @retval gfnInvalidParameter     - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment - If called in a cloud environment
    /// @retval gfnStreamFailure        - Network failure or other error prevented the stream from starting
    /// @retval gfnClientLibraryNotFound - GFN SDK client-side library could not be found.
    /// @retval gfnAPINotFound          - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnStartStream(StartStreamInput* startStreamInput, StartStreamResponse* response);

    ///
    /// @par Description
    /// Calls @ref gfnStartStreamAsync to request GFN client to start a streaming session of an
    /// application in an asynchronous fashion.
    ///
    /// @par Environment
    /// Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Use to start a streaming session.
    ///
    /// @param startStreamInput          - Pointer to a StartStreamInput structure.
    /// @param cb                        - A StartStreamCallbackSig callback to be called with streaming events.
    /// @param context                   - User context
    /// @param timeoutMs                 - Time after which attempt to start streaming will be aborted.
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a cloud environment
    /// @retval gfnClientLibraryNotFound - GFN SDK client-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    ///
    /// @note
    /// This function's definition differs from the export that it wraps, as it returns a
    /// GfnRuntimeError instead of being void for consistent error reporting.
    GfnRuntimeError GfnStartStreamAsync(const StartStreamInput* startStreamInput, StartStreamCallbackSig cb, void* context, unsigned int timeoutMs);

    ///
    /// @par Description
    /// Calls @ref gfnStopStream to request GFN client to stop a streaming session of an application
    /// in a synchronous (blocking) fashion.
    ///
    /// @par Environment
    /// Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Use to stop a streaming session started by the same process
    ///
    /// @retval gfnSuccess              - On success
    /// @retval gfnCallWrongEnvironment - If called in a cloud environment
    /// @retval gfnStreamStopFailure    - Failure to stop the streaming session
    /// @retval gfnClientLibraryNotFound - GFN SDK client-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnStopStream(void);

    ///
    /// @par Description
    /// Calls @ref gfnStopStreamAsync to request GFN client to stop a streaming session of an
    /// application in an asynchronous fashion.
    ///
    /// @par Environment
    /// Client
    /// 
    /// @par Platform
    /// Windows
    ///
    /// @par Usage
    /// Use to start a streaming session.
    ///
    /// @param cb                        - A StartStreamCallbackSig callback to be called with streaming events.
    /// @param context                   - User context
    /// @param timeoutMs                 - Time after which attempt to start streaming will be aborted.
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnStreamStopFailure     - Failure to stop the streaming session
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a cloud environment
    /// @retval gfnClientLibraryNotFound - GFN SDK client-side Library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    ///
    /// @note
    /// This function's definition differs from the export that it wraps, as it returns a
    /// GfnRuntimeError instead of being void for consistent error reporting.
    GfnRuntimeError GfnStopStreamAsync(StopStreamCallbackSig cb, void* context, unsigned int timeoutMs);

    ///
    /// @par Description
    /// Calls @ref gfnSetupTitle to notify GeForce NOW that an application should be readied for launch.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to prepare an application for launch on GeForce NOW, and block on the result.
    ///
    /// @param platformAppId             - Identifier of the requested application to setup
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnSetupTitleFailure     - The GeForce NOW SDK failed to set up the title
    /// @retval gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnSetupTitle(const char* platformAppId);

    ///
    /// @par Description
    /// Calls @ref gfnTitleExited to notify GFN that an application has exited.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to notify GFN that your application has exited.
    ///
    /// @param platformId                - Identifier of the launcher service, e.g. "Steam"
    /// @param platformAppId             - Identifier of the application that has exited.
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnInvalidParameter      - NULL pointer passed in
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnCloudLibraryNotFound  - GeForce NOW SDK cloud-side library could not be found
    /// @retval gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnTitleExited(const char* platformId, const char* platformAppId);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterExitCallback to register an application function that is called when
    /// the user has requested to end the GeForce NOW session.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Register an application function that is called when GeForce NOW will end the current session.
    /// Note: This callback function allows an application to respond to a user's request to end a session,
    /// for example, to perform a clean exit of the application. The application is given 5 seconds to 
    /// perform activities and return from the callback function before session shutdown activities continue.
    ///
    /// @param exitCallback             - Function pointer to application code to call when GeForce NOW
    ///                                   will begin to shut down the current session.
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   registered callback. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success, when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - If exitCallback function pointer is NULL
    /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
    ///                                   was called outside of a cloud execution environment
    /// @retval gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterExitCallback(ExitCallbackSig exitCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterPauseCallback to register an application callback with GeForce NOW
    /// to be called when GeForce NOW needs to pause the game on the user's behalf.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Register an application function to call when GeForce NOW needs to pause the game.
    /// Note: Callback should return as soon as the game process is paused. 
    /// Resuming should occur when the user interacts with the game UI.
    ///
    /// @param pauseCallback            - Function pointer to application code to call when
    ///                                   GeForce NOW needs to pause the game
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   registered callback. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success, when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - If pauseCallback function pointer is NULL
    /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
    ///                                   was called outside of a cloud execution environment
    /// @retval gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterInstallCallback to register an application callback with GeForce NOW
    /// to be called after a successful call to @ref gfnSetupTitle.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Register a function to call after a successful call to gfnSetupTitle.
    ///
    /// @param installCallback          - Function pointer to application code to call after
    ///                                   GeForce NOW successfully performs its own title setup.
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   registered callback. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success, when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - If installCallback function pointer is NULL
    /// @retval gfnCallWrongEnvironment - If callback could not be registered since this function
    ///                                   was called outside of a cloud execution environment
    /// @retval gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnRegisterInstallCallback(InstallCallbackSig installCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterSaveCallback to register an application callback with GeForce NOW to be
    /// called when GeForce NOW needs the application to save user progress.
    ///
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Register an application function to call when GeForce NOW needs the application to save game or user data.
    /// The callback function should trigger the save of game state and return when completed.
    ///
    /// @param saveCallback             - Function pointer to application code to call when GFN needs the application to save
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   callback specified. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success when running in a GeForce NOW environment
    /// @retval gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterSaveCallback(SaveCallbackSig saveCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterSessionInitCallback to register an application callback with GeForce NOW to be called when
    /// a GeForce NOW user has connected to the game seat to start a streaming session. This callback is used as part of
    /// the Pre-warm feature. 
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Register an application function to call when a GeForce NOW user has connected to the game seat to allow the game to load
    /// user-specific data and start game play for the user. Callback should return once loading is complete and user
    /// can interact with the game.
    /// Note: The callback function is given 30 second to complete and call @ref GfnAppReady. If this time elapses, before the 
    /// function is called, the session will begin streaming frames to the user even if the game is still loading user data.
    ///
    /// @param sessionInitCallback      - Function pointer to application code to call when the user has connected
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   callback specified. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - Callback was NULL
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnCallWrongEnvironment - The on-seat dll detected that it was not on a game seat
    GfnRuntimeError GfnRegisterSessionInitCallback(SessionInitCallbackSig sessionInitCallback, void* userContext);

    ///
    /// @par Description
    /// Calls @ref gfnRegisterMessageCallback to register an application callback with GeForce NOW to be called when a message
    /// is sent to the application via the SendMessage feature as part of GeForce NOW's support for two-way communication 
    /// between client application and the game application running in the game seat.
    ///
    /// @par Environment
    /// Cloud and Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Provide a callback function that will be called when a message is sent to the application from the SendMessage feature.
    ///
    /// @param messageCallback          - Function pointer to application code to call when a message has been sent.
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   callback specified. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - Callback was NULL
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnCallWrongEnvironment - The on-seat dll detected that it was not on a game seat
    GfnRuntimeError GfnRegisterMessageCallback(MessageCallbackSig messageCallback, void* userContext);


    ///
    /// @par Description
    /// Registers an application callback with GeForce NOW to be called when client info changes
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @param clientInfoCallback       - Function pointer to application code to call when GeForce NOW client data changes
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   callback specified. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - Callback was NULL
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterClientInfoCallback(ClientInfoCallbackSig clientInfoCallback, void* userContext);

    ///
    /// @par Description
    /// Registers an application callback with GeForce NOW to be called when network latency changes
    ///
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @param networkStatusCallback    - Function pointer to application code to call when network latency changes
    /// @param updateRateMs             - Time interval for updates
    /// @param userContext              - Pointer to user context, which will be passed unmodified to the
    ///                                   callback specified. Can be NULL.
    ///
    /// @retval gfnSuccess              - On success when running in a GeForce NOW environment
    /// @retval gfnInvalidParameter     - Callback was NULL
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterNetworkStatusCallback(NetworkStatusCallbackSig networkStatusCallback, unsigned int updateRateMs, void* userContext);

    ///
    /// @par Description
    /// Calls @ref GfnAppReady to notify GeForce NOW that an application is ready to be displayed to the GeForce NOW user.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to notify GFN that your application is ready to be streamed.
    ///
    /// @param success                - True if startup was successful
    /// @param status                 - Optional startup status
    ///
    /// @retval gfnSuccess               - On success
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnAPINotInit            - SDK was not initialized
    /// @retval gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnAppReady(bool success, const char * status);

    ///
    /// @par Description
    /// Sends Active zone coordinates to GFN Client
    ///
    /// TO BE DEPRECATED SOON
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to invoke special events on the client from the GFN cloud environment
    /// Note : Marked for deprecation
    ///
    /// @param type                 - Populated with relevant GfnActionType
    /// @param id                   - unique unsigned int type identifier for this event
    /// @param zone                 - Optional zone coordinates relevant to type specified
    ///
    /// @retval gfnSuccess              - Call was successful
    /// @retval gfnInputExpected        - Expected zone to have a value
    /// @retval gfnComError             - There was SDK internal communication error
    /// @retval gfnCallWrongEnvironment  - If called in a client environment
    /// @retval gfnAPINotInit            - SDK was not initialized
    /// @retval gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnInvalidParameter     - Invalid parameters provided
    /// @retval gfnThrottled            - API call was throttled for exceeding limit
    /// @retval gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    /// @return Otherwise, appropriate error code
    GfnRuntimeError GfnSetActionZone(GfnActionType type, unsigned int id, GfnRect* zone);


    ///
    /// @par Description
    /// Sends a custom message communication from the app to the client. This message
    /// is "fire-and-forget", and does not wait for the message to be delivered to return status.
    /// Latency is best effort and not guaranteed.
    ///
    /// @par Environment
    /// Cloud or Client
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to communicate between cloud applications and streaming clients.
    ///
    /// @param pchMessage - Character string
    /// @param length     - Length of pchMessage in characters, which cannot exceed 8K in length
    ///
    /// @retval gfnSuccess              - Call was successful
    /// @retval gfnComError             - There was SDK internal communication error
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnInvalidParameter     - Invalid parameters provided, or message exceeded allowed length
    /// @retval gfnThrottled            - API call was throttled for exceeding limit of 30 messages per second
    /// @retval gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    /// @retval gfnCloudLibraryNotFound - GFN SDK cloud-side library could not be found
    /// @return Otherwise, appropriate error code
    GfnRuntimeError GfnSendMessage(const char* pchMessage, unsigned int length);
    ///
    /// @par Description
    /// Requests the client application to open a URL in their local web browser.
    /// 
    /// These requests are fire-and-forget. Typically, the client will see a popup during their streaming session with a link that
    /// they can click to open the local web browser.
    /// 
    /// Usage of this API is limited to  5 URL requests every second, with no more than 25 URL requests per minute.
    ///
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to request that the client application open a URL link in their local web browser.
    ///
    /// @param pchUrl - A URL which should be opened on the client's local browser.
    ///
    /// @retval gfnSuccess              - Call was successful
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnCallWrongEnvironment - API was called outside of a cloud execution environment
    /// @retval gfnInvalidParameter     - Invalid pointer provided, or message exceeded allowed length
    /// @retval gfnThrottled            - API call was throttled for exceeding limit of 5 messages per second or 25 total per session
    /// @retval gfnInternalError        - API ran into an internal error
    /// @return Otherwise, appropriate error code
    GfnRuntimeError GfnOpenURLOnClient(const char* pchUrl);

    ///
    /// @par Description
    /// Allows the application or game to inform GFN of its overall running state
    /// 
    /// @par Environment
    /// Cloud
    /// 
    /// @par Platform
    /// Windows, Linux
    ///
    /// @par Usage
    /// Use to inform GFN when the application enters a specific running state to allow GFN to make certain decisions about the session
    ///
    /// @param appState - A @ref GfnAppState value
    ///
    /// @retval gfnSuccess              - Call was successful
    /// @retval gfnAPINotInit           - SDK was not initialized
    /// @retval gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    /// @retval gfnCallWrongEnvironment - API was called outside of a cloud execution environment
    /// @retval gfnInternalError        - API ran into an internal error
    /// @return Otherwise, appropriate error code
    GfnRuntimeError GfnSetAppState(GfnAppState appState);

    /// @}
#ifdef __cplusplus
    } // extern "C"
#endif
