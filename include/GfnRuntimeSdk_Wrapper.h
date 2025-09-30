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
// Copyright (c) 2020-2025 NVIDIA Corporation. All rights reserved.

//
// ===============================================================================================
//
// C-based wrapper API declarations for GFN Runtime SDK APIs
//
// ===============================================================================================
//
// Native Runtime API Export Wrapper Reference
//
// Introduction
// The NVIDIA GFN Runtime SDK provides a set of APIs as export functions from a dynamic
// library. To ease integration and use of these APIs, the GFN Runtime SDK provides
// this optional set of pure-C functions that manage the lifetime of the library as well as
// wrap the export function calls. This allows integrators to avoid having to work with
// export functions directly if they so choose.
//
// Most of the functions defined in this header file have the same definitions and calling
// characteristics as the export function they wrap. As such, they will return the same
// data types as well as error codes. As such, the documentation for each is purposefully
// pared down to avoid duplication. There are a few exceptions, and those functions will have
// more extensive documentation that calls out the differences.
//

#include "GfnRuntimeSdk_CAPI.h"
#include <stddef.h>

#ifdef _WIN32
    #define CHAR_TYPE wchar_t
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif __linux__
    #define CHAR_TYPE char
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    // Description
    // Loads the GFN SDK dynamic library and calls gfnInitializeRuntimeSdk. By default, the function
    // expects the library to be in the same folder as the loading process's executable. For
    // security reasons, the dynamic library is loaded by fully-quantified path. If the GFN SDK
    // library is packaged in another folder, you will need to locally modify the function to
    // reference that location.
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call as soon as possible during application startup.
    //
    // Parameters
    // none
    // 
    // Return Values
    // gfnSuccess                - If the SDK was initialized and all SDK features are available.
    // gfnInitSuccessClientOnly  - If the SDK was initialized, but only client-side functionality is available.
    // gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    // gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    // gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnInitializeSdk(void);

    //
    // Description
    // Calls gfnShutdownRuntimeSdk to releases the SDK and resources and disconnects from GFN
    // backend and then unloads unloads the GFN SDK Library.
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call during application shutdown or when GFN Runtime API methods are no longer needed.
    // 
    // Parameters
    // none
    // 
    // Return Values
    // gfnSuccess                - If the SDK was initialized and all SDK features are available
    // gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnShutdownSdk(void);

    //
    // Description
    // Calls gfnIsRunningInCloud to determine if calling application is running in GFN environment,
    // and without requiring process registration.
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to quickly determine whether to enable / disable any low-value GFN cloud environment
    // specific application logic, for example to know if GfnGetTitlesAvailable can be called without an error.
    //
    // This API is meant to fill the need to quickly determine if the call looks to be in the
    // GFN cloud environment. It purposefully trades off resource-intensive checks for fast response.
    // Do not tie any logic or features of value to this API as the call stack could be tampered with.
    // For that purpose, use GfnIsRunningInCloudSecure.
    //
    // This function's definition differs from the export that it wraps, as it returns a GfnRuntimeError
    // instead of bool
    // 
    // Parameters
    // runningInCloud             - Pointer to a boolean that receives true if running in GeForce NOW
    //                                     cloud or false if not in the GeForce NOW cloud
    // 
    // Return Values
    // gfnSuccess                - If the query was successful
    // gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    // gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    // gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnIsRunningInCloud(bool* runningInCloud);

    //
    // Description
    // Calls gfnIsRunningInCloudSecure to determine if calling application is running in GFN environment,
    // and what level of security assurance that the result is valid.
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows
    //
    // Usage
    // Call from an NVIDIA-approved process to securely determine whether running in GFN cloud, and use the
    // GfnIsRunningInCloudAssurance value to decide the risk to enable any application specific logic
    // for that environment.
    //
    // This API must be called from a process that has been registered with NVIDIA, or it will return an error.
    // Refer to the Cloud Check API Guide on how to get your application registered. To prevent
    // man-in-the-middle (MiM) attacks, you must also securely load the SDK library, checking the integrity
    // of the digital signature on the binary. Make sure to use the value returned from GfnIsRunningInCloudAssurance
    // to decide if the check was certain enough without tampering to enable the logic or feature associated
    // with the API call.
    //
    // Parameters
    // assurance                  - Likelihood and level of security assurance defined via GfnIsRunningInCloudAssurance 
    //                                    that API is running in GFN cloud environment
    //
    // Return Values
    // gfnSuccess                - If the query was successful.
    // gfnClientLibraryNotFound  - GFN SDK client-side library could not be found
    // gfnCloudLibraryNotFound   - GFN SDK cloud-side library could not be found
    // gfnAPINotFound            - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnIsRunningInCloudSecure(GfnIsRunningInCloudAssurance* assurance);

    //
    // Description
    // Determines if calling application is running in GFN environment or not. It also provides a 
    // cryptographically signed response if requested, which can be verified either locally or via 3rd party backend.
    // Refer to the Cloud Check API Guide on how to validate attestation data in response.
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // This API can be used from any execution context - privileged or not. 
    //
    // Parameters
    // challenge                 - Optional input parameter, that can be used to pass in nonce data.
    //                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    // 
    // response                  - Optional output parameter, that receives the signed attestation response from the API.
    // 
    // isCloudEnvironment        - Optional output parameter, that receives true value if the caller is in the GFN environment.
    //
    // Return Values
    // gfnSuccess               - On success indicates cloud check was performed successfully.
    // gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    // gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    //                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    // gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    // gfnThrottled             - API call was throttled for exceeding limit
    GfnRuntimeError GfnCloudCheck(const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, bool* isCloudEnvironment);

    //
    // Description
    // Determines which GFN environment, if any, is available. This is not useful for most developers.
    // Only use if you've been instructed to by NVIDIA support.
    // 
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows
    //
    // Usage
    // This API can be used from any execution context - privileged or not. 
    //
    // Parameters
    // requested_cloud_type      - The desired type of the cloud (CC_CLOUD_TYPE_TRUSTED, CC_CLOUD_TYPE_OPEN, or CC_CLOUD_TYPE_ANY) that the caller wants to check for.
    // challenge                 - Optional input parameter, that can be used to pass in nonce data.
    //                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    // 
    // response                  - Optional output parameter, that receives the signed attestation response from the API.
    // 
    // detected_cloud_type       - An optional parameter to receive the actual type of the environment detected at runtime (CC_CLOUD_TYPE_xxx).
    //                                    This parameter is only useful when multiple cloud types were provided on input in the requested_cloud_type parameter.
    //                                    A null pointer can be passed if the caller is not interested in receiving the actual cloud type.
    //
    // Return Values
    // gfnSuccess               - On success indicates cloud check was performed successfully.
    // gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    // gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    //                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    // gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    // gfnUnsupportedAPICall    - When called from a Linux environment.
    // gfnThrottled             - API call was throttled for exceeding limit
    GfnRuntimeError GfnGetCloudType(
        const GfnCloudType requested_cloud_type,
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response,
        GfnCloudType* detected_cloud_type);

    //
    // Description
    // Determines if calling application is running in the "trusted" cloud environment.
    //
    // This call is functionally equivalent to GfnGetCloudType(CC_CLOUD_TYPE_TRUSTED, &challenge, &response, &detectedCloudEnvironment);
    // 
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // This API can be used from any execution context - privileged or not. 
    //
    // Parameters
    // challenge                 - Optional input parameter, that can be used to pass in nonce data.
    //                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    // 
    // response                  - Optional output parameter, that receives the signed attestation response from the API.
    //
    // Return Values
    // gfnSuccess               - On success indicates cloud check was performed successfully.
    // gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    // gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    //                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    // gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    // gfnThrottled             - API call was throttled for exceeding limit
    // gfnCallWrongEnvironment  - The call succeeded, but the application is not executing in the trusted cloud environment.
    GfnRuntimeError GfnGetTrustedCloud(
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response);

    // Description
    // Determines if calling application is running in the "open" cloud environment.
    // This is not useful for most developers.
    // Only use if you've been instructed to by NVIDIA support.
    //
    // This call is functionally equivalent to GfnGetCloudType(CC_CLOUD_TYPE_OPEN, &challenge, &response, &actualCloudEnvironment);
    //
    // Environment
    // Cloud and Client
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // This API can be used from any execution context - privileged or not. 
    //
    // Parameters
    // challenge                 - Optional input parameter, that can be used to pass in nonce data.
    //                                    If a non-null challenge is passed in, then the response parameter is mandatory.
    // 
    // response                  - Optional output parameter, that receives the signed attestation response from the API.
    //
    // Return Values
    // gfnSuccess               - On success indicates cloud check was performed successfully.
    // gfnInvalidParameter      - NULL pointer passed in response parameter when challenge parameter is nonzero.
    // gfnNotAuthorized         - Indicates the application is either not properly onboarded (missing allow - list),
    //                                    or the application attempted to perform cloud check in an unsafe environment (patched game seat).
    // gfnBackendError          - Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment.
    // gfnThrottled             - API call was throttled for exceeding limit
    // gfnCallWrongEnvironment  - The call succeeded, but the application is not executing in the open cloud environment.
    GfnRuntimeError GfnGetOpenCloud(
        const GfnCloudCheckChallenge* challenge,
        GfnCloudCheckResponse* response);
    //
    // Description
    // Calls gfnGetClientIp to get user client's IP address.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call this during application start or from the platform client in
    // order to get the user's external client IP address.
    //
    // Parameters
    // clientIp - Output IPv4 in string format. Example: "192.168.0.1". Call GfnFree to free the memory.
    //
    // Return Values
    // gfnSuccess               - Successfully retrieved client IP
    // gfnNoData                - No IP data found
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetClientIpV4(const char** clientIp);

    //
    // Description
    // Calls gfnGetClientLanguageCode to gets user's client language code in the form
    // "<lang>-<country>" using a standard RFC3066 language locale code (example: en_US) and
    // ISO 3166-1 Alpha-2 country code (example: GB).
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call this during application start or from the platform client in
    // order to get the user's language locale and country settings.
    //
    // Parameters
    // languageCode - Language code as a string. Example: "en_US-GB".
    //                Call GfnFree to free the memory.
    //
    // Return Values
    // gfnSuccess               - Successfully retrieved language code
    // gfnNoData                - No language code found
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetClientLanguageCode(const char** languageCode);

    //
    // Description
    // Gets user's client country code using ISO 3166-1 Alpha-2 country code.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call this during application start or from the platform client in order to get
    // the user's country code.
    //
    // Parameters
    // countryCode              - Country code as a 2 character string. Example: “US”
    // length                   - Length of pchCountryCode character array
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetClientCountryCode(char* countryCode, unsigned int length);

    //
    // Description
    // Gets user's client data.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call this during application start or from the platform client in order to get
    // the data on the user's client.
    // 
    // The client data returned by this function is best effort, as not all client devices will report
    // all information. For example, certain browser and TV clients will not report client resolution or
    // will incorrect resolution in certain situations. In the cases the data is not reported, the data
    // returned will be zeroes. Likewise for safezone information, as most clients, such as Windows PC,
    // do not have screen areas unsafe for rendering or input. For more information about safe zones,
    // see the  the Mobile Integration Guide in the /doc folder.
    //
    // Parameters
    // clientInfo               - A structure to hold clientInfo data
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetClientInfo(GfnClientInfo* clientInfo);

    //
    // Description
    // Gets various information about the current streaming session
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Call this from a streaming session to find out more information about the session, such
    // as session time remaining, or if RTX is enabled for the current session.
    //
    // If the application has called gfnRegisterSessionInitCallback to be notified when a
    // user connects, then this API should be called after that callback is triggered.
    // Certain data, such as session time limit or RTX support, can only be defined when a user
    // connects as the values depend on the user type. Calling before that point can result in
    // obtaining incorrect data.
    // 
    // Parameters
    // sessionInfo               - Pointer to a GfnSessionInfo struct.
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in or buffer length is too small
    // gfnCallWrongEnvironment  - If called in a client environment
    GfnRuntimeError GfnGetSessionInfo(GfnSessionInfo* sessionInfo);

    //
    // Description
    // Calls GfnGetPartnerData to retrieves non-secure partner data that is passed into the session.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use during cloud session to retrieve partner data
    //
    // Parameters
    // partnerData              - Populated with the partner data in string form if found
    //                                   Call GfnFree to free the memory
    //
    // Return Values
    // gfnSuccess              - Partner data successfully retrieved from session data
    // gfnNoData               - No partner data found in session data
    // gfnInvalidParameter     - NULL pointer passed in
    // gfnCallWrongEnvironment - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound          - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetPartnerData(const char** partnerData);

    //
    // Description
    // Calls GfnGetPartnerSecureData to retrieves secure partner data that is passed into the session.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use during cloud session to retrieve secure partner data
    //
    // Parameters
    // partnerSecureData         - Populated with the secure partner data in string for if found
    //                                    Call GfnFree to free the memory
    //
    // Return Values
    // gfnSuccess               - Secure partner data successfully retrieved from session data
    // gfnNoData                - No secure partner data found in session data
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetPartnerSecureData(const char** partnerSecureData);

    //
    // Description
    // Calls gfnIsTitleAvailable to determines if a specific title is available to launch
    // in current streaming session.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to determine if a title is available to be launched from the active GFN cloud instance,
    // for example, to show a "Play" button in a platform launcher's UI.
    //
    // This function's definition differs from the export that it wraps, as it returns a
    // GfnRuntimeError instead of bool.
    // 
    // Parameters
    // platformAppId             - Identifier of the requested title to check
    // isAvailable               - Pointer to a boolean that receives true if the title is
    //                                    available, or or false if not available.
    // 
    // Return Values
    // gfnSuccess               - If the query was successful.
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnIsTitleAvailable(const char* platformAppId, bool* isAvailable);

    //
    // Description
    // Calls GfnGetTitlesAvailable to retrieves all titles that can be launched in the
    // current game streaming session.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to retrieve a list of all titles available to launch in the current streaming session,
    // for example, to add "Play" buttons to all titles instead of calling gfnIsTitleAvailable on
    // each title.
    //
    // Parameters
    // platformAppIds            - Comma-delimited list of platform identifiers. Memory is
    //                                    allocated for the list. Call GfnFree to free the memory.
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnGetTitlesAvailable(const char** platformAppIds);

    //
    // Description
    // Calls gfnFree to free memory allocated by GfnGetTitlesAvailable
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to release memory after a call to a memory-allocated function and you are finished with the data.
    // Should only be called if the memory is populated with valid data. Calling GfnFree with invalid
    // pointer or data will result in an memory exception being thrown.
    //
    // Parameters
    // data                      - Pointer to allocated string memory
    //
    // Return Values
    // gfnSuccess               - Memory successfully released
    // gfnInvalidParameter      - NULL pointer passed in
    GfnRuntimeError GfnFree(const char** data);

    //
    // Description
    // Calls gfnSetupTitle to notify GeForce NOW that an application should be readied for launch.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to prepare an application for launch on GeForce NOW, and block on the result.
    //
    // Parameters
    // platformAppId             - Identifier of the requested application to setup
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnSetupTitleFailure     - The GeForce NOW SDK failed to set up the title
    // gfnCloudLibraryNotFound  - GFN SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnSetupTitle(const char* platformAppId);

    //
    // Description
    // Calls gfnTitleExited to notify GFN that an application has exited.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to notify GFN that your application has exited.
    //
    // Parameters
    // platformId                - Identifier of the launcher service, e.g. "Steam"
    // platformAppId             - Identifier of the application that has exited.
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnInvalidParameter      - NULL pointer passed in
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnCloudLibraryNotFound  - GeForce NOW SDK cloud-side library could not be found
    // gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnTitleExited(const char* platformId, const char* platformAppId);

    //
    // Description
    // Calls gfnRegisterExitCallback to register an application function that is called when
    // the user has requested to end the GeForce NOW session.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Register an application function that is called when GeForce NOW will end the current session.
    // Note: This callback function allows an application to respond to a user's request to end a session,
    // for example, to perform a clean exit of the application. The application is given 5 seconds to 
    // perform activities and return from the callback function before session shutdown activities continue.
    //
    // Parameters
    // exitCallback             - Function pointer to application code to call when GeForce NOW
    //                                   will begin to shut down the current session.
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   registered callback. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success, when running in a GeForce NOW environment
    // gfnInvalidParameter     - If exitCallback function pointer is NULL
    // gfnCallWrongEnvironment - If callback could not be registered since this function
    //                                   was called outside of a cloud execution environment
    // gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterExitCallback(ExitCallbackSig exitCallback, void* userContext);

    //
    // Description
    // Calls gfnRegisterPauseCallback to register an application callback with GeForce NOW
    // to be called when GeForce NOW needs to pause the game on the user's behalf.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Register an application function to call when GeForce NOW needs to pause the game.
    // Note: Callback should return as soon as the game process is paused. 
    // Resuming should occur when the user interacts with the game UI.
    //
    // Parameters
    // pauseCallback            - Function pointer to application code to call when
    //                                   GeForce NOW needs to pause the game
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   registered callback. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success, when running in a GeForce NOW environment
    // gfnInvalidParameter     - If pauseCallback function pointer is NULL
    // gfnCallWrongEnvironment - If callback could not be registered since this function
    //                                   was called outside of a cloud execution environment
    // gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* userContext);

    //
    // Description
    // Calls gfnRegisterInstallCallback to register an application callback with GeForce NOW
    // to be called after a successful call to gfnSetupTitle.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Register a function to call after a successful call to gfnSetupTitle.
    //
    // Parameters
    // installCallback          - Function pointer to application code to call after
    //                                   GeForce NOW successfully performs its own title setup.
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   registered callback. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success, when running in a GeForce NOW environment
    // gfnInvalidParameter     - If installCallback function pointer is NULL
    // gfnCallWrongEnvironment - If callback could not be registered since this function
    //                                   was called outside of a cloud execution environment
    // gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    // gfnAPINotFound          - The API was not found in the GFN SDK Library
    GfnRuntimeError GfnRegisterInstallCallback(InstallCallbackSig installCallback, void* userContext);

    //
    // Description
    // Calls gfnRegisterSaveCallback to register an application callback with GeForce NOW to be
    // called when GeForce NOW needs the application to save user progress.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Register an application function to call when GeForce NOW needs the application to save game or user data.
    // The callback function should trigger the save of game state and return when completed.
    //
    // Parameters
    // saveCallback             - Function pointer to application code to call when GFN needs the application to save
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   callback specified. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success when running in a GeForce NOW environment
    // gfnCloudLibraryNotFound - GeForce NOW SDK cloud-side library could not be found
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterSaveCallback(SaveCallbackSig saveCallback, void* userContext);

    //
    // Description
    // Calls gfnRegisterSessionInitCallback to register an application callback with GeForce NOW to be called when
    // a GeForce NOW user has connected to the game seat to start a streaming session. This callback is used as part of
    // the Pre-warm feature. 
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Register an application function to call when a GeForce NOW user has connected to the game seat to allow the game to load
    // user-specific data and start game play for the user. Callback should return once loading is complete and user
    // can interact with the game.
    // Note: The callback function is given 30 second to complete and call GfnAppReady. If this time elapses, before the 
    // function is called, the session will begin streaming frames to the user even if the game is still loading user data.
    //
    // Parameters
    // sessionInitCallback      - Function pointer to application code to call when the user has connected
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   callback specified. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success when running in a GeForce NOW environment
    // gfnInvalidParameter     - Callback was NULL
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnCallWrongEnvironment - The on-seat dll detected that it was not on a game seat
    GfnRuntimeError GfnRegisterSessionInitCallback(SessionInitCallbackSig sessionInitCallback, void* userContext);

    //
    // Description
    // Calls gfnRegisterMessageCallback to register an application callback with GeForce NOW to be called when a message
    // is sent to the application via the SendMessage feature as part of GeForce NOW's support for two-way communication 
    // between client application and the game application running in the game seat.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Provide a callback function that will be called when a message is sent to the application from the SendMessage feature.
    //
    // Parameters
    // messageCallback          - Function pointer to application code to call when a message has been sent.
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   callback specified. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success when running in a GeForce NOW environment
    // gfnInvalidParameter     - Callback was NULL
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnCallWrongEnvironment - The on-seat dll detected that it was not on a game seat
    GfnRuntimeError GfnRegisterMessageCallback(MessageCallbackSig messageCallback, void* userContext);

    //
    // Description
    // Registers an application callback with GeForce NOW to be called when client info changes
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Parameters
    // clientInfoCallback       - Function pointer to application code to call when GeForce NOW client data changes
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   callback specified. Can be NULL.
    // Return Values
    // gfnSuccess              - On success when running in a GeForce NOW environment
    // gfnInvalidParameter     - Callback was NULL
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterClientInfoCallback(ClientInfoCallbackSig clientInfoCallback, void* userContext);

    //
    // Description
    // Registers an application callback with GeForce NOW to be called when network latency changes
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Parameters
    // networkStatusCallback    - Function pointer to application code to call when network latency changes
    // updateRateMs             - Time interval for updates
    // userContext              - Pointer to user context, which will be passed unmodified to the
    //                                   callback specified. Can be NULL.
    //
    // Return Values
    // gfnSuccess              - On success when running in a GeForce NOW environment
    // gfnInvalidParameter     - Callback was NULL
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnRegisterNetworkStatusCallback(NetworkStatusCallbackSig networkStatusCallback, unsigned int updateRateMs, void* userContext);

    //
    // Description
    // Calls GfnAppReady to notify GeForce NOW that an application is ready to be displayed to the GeForce NOW user.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to notify GFN that your application is ready to be streamed.
    //
    // Parameters
    // success                - True if startup was successful
    // status                 - Optional startup status
    //
    // Return Values
    // gfnSuccess               - On success
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnAPINotInit            - SDK was not initialized
    // gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    GfnRuntimeError GfnAppReady(bool success, const char * status);

    //
    // Description
    // Sends Active zone coordinates to GFN Client
    //
    // TO BE DEPRECATED SOON
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to invoke special events on the client from the GFN cloud environment
    // Note : Marked for deprecation
    //
    // Parameters
    // type                 - Populated with relevant GfnActionType
    // id                   - unique unsigned int type identifier for this event
    // zone                 - Optional zone coordinates relevant to type specified
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnInputExpected        - Expected zone to have a value
    // gfnComError             - There was SDK internal communication error
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnAPINotInit            - SDK was not initialized
    // gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    // gfnInvalidParameter     - Invalid parameters provided
    // gfnThrottled            - API call was throttled for exceeding limit
    // gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    GfnRuntimeError GfnSetActionZone(GfnActionType type, unsigned int id, GfnRect* zone);

    //
    // Description
    // Sends Edit-Box region coordinates and parameters to GFN Client
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Used for marking a textbox region for monitoring touch at client to invoke soft keyboard
    //
    // Parameters
    // id        - unique unsigned int type identifier for this EditBox region
    // region    - Region coordinates of the EditBox
    // flags     - Bitflag for EditBox region flags
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnInputExpected        - Expected zone to have a value
    // gfnComError             - There was SDK internal communication error
    // gfnCallWrongEnvironment  - If called in a client environment
    // gfnAPINotInit            - SDK was not initialized
    // gfnAPINotFound           - The API was not found in the GeForce NOW SDK Library
    // gfnInvalidParameter     - Invalid parameters provided
    // gfnThrottled            - API call was throttled for exceeding limit
    // gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    GfnRuntimeError GfnSetEditBoxRegion(unsigned int id, GfnRect* region, unsigned int flags);

    //
    // Description
    // Clears the EditBox region specified using unique id
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to clear an Edit Box region specified earlier using SetEditBoxRegion API
    //
    // Parameters
    // id - unique id of EditBox region that is to be cleared
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnInputExpected        - Expected zone to have a value
    // gfnComError             - There was SDK internal communication error
    // gfnCallWrongEnvironment - If called in a client environment
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnInvalidParameter     - Invalid parameters provided
    // gfnThrottled            - API call was throttled for exceeding limit
    // gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    GfnRuntimeError GfnClearEditBoxRegion(unsigned int id);

    //
    // Description
    // Sends a custom message communication from the app to the client. This message
    // is "fire-and-forget", and does not wait for the message to be delivered to return status.
    // Latency is best effort and not guaranteed.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to communicate between cloud applications and streaming clients.
    //
    // Parameters
    // pchMessage - Character string
    // length     - Length of pchMessage in characters, which cannot exceed 8K in length
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnComError             - There was SDK internal communication error
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnInvalidParameter     - Invalid parameters provided, or message exceeded allowed length
    // gfnThrottled            - API call was throttled for exceeding limit of 30 messages per second
    // gfnUnhandledException   - API ran into an unhandled error and caught an exception before it returned to client code
    // gfnCloudLibraryNotFound - GFN SDK cloud-side library could not be found
    GfnRuntimeError GfnSendMessage(const char* pchMessage, unsigned int length);
    //
    // Description
    // Requests the client application to open a URL in their local web browser.
    // 
    // These requests are fire-and-forget. Typically, the client will see a popup during their streaming session with a link that
    // they can click to open the local web browser.
    // 
    // Usage of this API is limited to  5 URL requests every second, with no more than 25 URL requests per minute.
    //
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to request that the client application open a URL link in their local web browser.
    //
    // Parameters
    // pchUrl - A URL which should be opened on the client's local browser.
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnCallWrongEnvironment - API was called outside of a cloud execution environment
    // gfnInvalidParameter     - Invalid pointer provided, or message exceeded allowed length
    // gfnThrottled            - API call was throttled for exceeding limit of 5 messages per second or 25 total per session
    // gfnInternalError        - API ran into an internal error
    GfnRuntimeError GfnOpenURLOnClient(const char* pchUrl);

    //
    // Description
    // Allows the application or game to inform GFN of its overall running state
    // 
    // Environment
    // Cloud
    // 
    // Platform
    // Windows, Linux
    //
    // Usage
    // Use to inform GFN when the application enters a specific running state to allow GFN to make certain decisions about the session
    //
    // Parameters
    // appState - A GfnAppState value
    //
    // Return Values
    // gfnSuccess              - Call was successful
    // gfnAPINotInit           - SDK was not initialized
    // gfnAPINotFound          - The API was not found in the GeForce NOW SDK Library
    // gfnCallWrongEnvironment - API was called outside of a cloud execution environment
    // gfnInternalError        - API ran into an internal error
    GfnRuntimeError GfnSetAppState(GfnAppState appState);


#ifdef __cplusplus
    } // extern "C"
#endif
