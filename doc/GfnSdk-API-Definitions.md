# GeForce NOW Software Development Kit: API Definition Guide

| Version | Date | Description of Change | 
| :----: | :----: | :---- |
| 1.0 | 07/11/2025 | First revision |

## Introduction
NVIDIA GeForce NOW (GFN) allows users to enjoy their library of PC games on many internet-enabled devices via the power of cloud gaming. This includes PCs, Macs, Smart TVs, and mobile devices such as smartphones and tablets. 

GFN has the ability to onboard your Windows and Linux games and applications as-is for your users to enjoy. For tighter integration of these applications with GFN, NVIDIA provides the GeForce NOW Runtime Software Development Kit (GFN SDK) to developers so that their applications can perform various tasks; from knowing when the application is running in GFN to getting information about the GFN session or the user’s client system.

This document is provided as part of the SDK to aid in getting the developer up to speed with the SDK quickly, as well as providing source code pointers for the most common of integration scenarios.

## APIs

### Initialize and Shutdown Functions
The functions in this section of the document are used to initialize the SDK before any other API functions are called, and to shutdown the SDK to allow for resource cleanup when a title no longer needs to use the GFN SDK.

### gfnInitializeRuntimeSdk

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnInitializeRuntimeSdk(void);

```

Should be called at application startup and prior to any other GFN Runtime API methods.
The SDK features which become available as a result of initializing the GeForce NOW
SDK are dependent on the type of environment in which the SDK operates (client or cloud).

Environment: Cloud and Client   
Platform: Windows, Linux   
Parameters: None   
Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | If the SDK was initialized and all SDK features are available. |
| gfnInitSuccessClientOnly | If the SDK was initialized, but only client-side functionality is available, such as calls to gfnIsRunningInCloud or gfnCloudCheck, but are expected to return false in this scenario. |
| gfnUnableToAllocateMemory | SDK initialization failed due to lack of available memory, and no SDK functionality is available. |
| gfnInitFailure | SDK initialization failed for any other reason, and no SDK functionality is available. |

### gfnShutdownRuntimeSdk

```c

NVGFNSDK_EXPORT void NVGFNSDKApi gfnShutdownRuntimeSdk(void);

```

Releases the SDK, and frees up memory allocated by GFN and disconnects from GFN backend.

Environment: Cloud and Client   
Platform: Windows, Linux   
Parameters: None   
Return Values: None   

### GeForce NOW Environment Check Functions
The functions in this section of the document provide ways for applications to determine if they are executing inside the GeForce NOW Cloud environment.

### gfnIsRunningInCloud

```c

NVGFNSDK_EXPORT bool NVGFNSDKApi gfnIsRunningInCloud(void);

```

Quickly determines if the calling application is running in GFN environment or not with a low security assurance, and without requiring process registration.

Environment: Cloud and Client   
Platform: Windows, Linux   
Parameters: None   
Return Values:   
| Value | Meaning |
| :---- | :---- |
| true | Application is running on a GFN game seat virtual machine in the GeForce NOW cloud. |
| false | Application is not running in the GeForce NOW cloud or in a GeForce NOW test environment. This value will also be returned if called before gfnInitializeRuntimeSdk(). |

### gfnIsRunningInCloudSecure

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnIsRunningInCloudSecure(GfnIsRunningInCloudAssurance* assurance);

```

Determines if calling application is running in GFN environment or not, and what degree of security assurance is assigned to the result.

Environment: Cloud and Client   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| assurance | Level of assurance that API is running in GFN cloud environment |
Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |


### gfnCloudCheck

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnCloudCheck(const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, bool* isCloudEnvironment);

```

Determines if calling application is running in GFN environment or not. It also provides a cryptographically signed response if requested, which can be verified either locally or via 3rd party backend.
Refer to the Cloud Check API Guide on how to validate attestation data in response.

Environment: Cloud and Client    
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| challenge | Optional input parameter, that can be used to pass in nonce data. If a non-null challenge is passed in, then the response parameter is mandatory. |
| response | Optional output parameter, that receives the signed attestation response from the API. |
| isCloudEnvironment | Optional output parameter, that receives true value if the caller is in the GFN environment. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in response parameter when challenge parameter is nonzero. |
| gfnNotAuthorized | Indicates the application is either not properly onboarded (missing allow - list), or the application attempted to perform cloud check in an unsafe environment (patched game seat). |
| gfnBackendError | Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment. |
| gfnThrottled | API call was throttled for exceeding limit. |




### gfnGetCloudType

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnGetCloudType(const GfnCloudType requested_cloud_type, const GfnCloudCheckChallenge* challenge, GfnCloudCheckResponse* response, GfnCloudType* detected_cloud_type);

```

Determines which GFN environment, if any, is available. This is not useful for most developers. Only use if you've been instructed to by NVIDIA support.

Environment: Cloud and Client   
Platform: Windows   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| requested_cloud_type | The desired type of the cloud (CC_CLOUD_TYPE_TRUSTED, CC_CLOUD_TYPE_OPEN, or CC_CLOUD_TYPE_ANY) that the caller wants to check for. |
| challenge | Optional input parameter, that can be used to pass in nonce data. If a non-null challenge is passed in, then the response parameter is mandatory. |
| response | Optional output parameter, that receives the signed attestation response from the API. |
| detected_cloud_type | An optional parameter to receive the actual type of the environment detected at runtime (CC_CLOUD_TYPE_xxx). This parameter is only useful when CC_CLOUD_TYPE_ANY was provided on input in the requested_cloud_type parameter. A null pointer can be passed if the caller is not interested in receiving the actual cloud type. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in response parameter when challenge parameter is nonzero. |
| gfnNotAuthorized | Indicates the application is either not properly onboarded (missing allow - list), or the application attempted to perform cloud check in an unsafe environment (patched game seat). |
| gfnBackendError | Indicates the API could not communicate with the GFN backend services to confirm it is running in GFN environment. |
| gfnUnsupportedAPICall | When called from a Linux environment. |
| gfnThrottled | API call was throttled for exceeding limit. |


### Data Retrieval Functions
The functions in this section of the document focus on obtaining data about the GFN session or the client device that requested the session, and can only be called from a GFN session.

### gfnIsTitleAvailable

```c

NVGFNSDK_EXPORT bool NVGFNSDKApi gfnIsTitleAvailable(const char* pchPlatformAppId);

```

Determines if a specific title is available to launch in current streaming session.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchPlatformAppId | Identifier of the requested title to check |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| true | Title is available for playing. |
| false | Title is not available for playing. The common cases for this is the title is not supported on GFN, or the GFN servers do not load every supported game to every system in all cases. |

### gfnGetTitlesAvailable

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetTitlesAvailable(const char** ppchPlatformAppIds);

```

Retrieves all titles that can be launched in the current game streaming session.

Environment: Cloud   
Platform: Windows, Linux     
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchPlatformAppIds | Comma-delimited list of platform identifiers. Memory is allocated for the list. Call gfnFree to release the memory when done. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in response parameter when challenge parameter is nonzero. |
| gfnCallWrongEnvironment | Function called in a client environment. |

### gfnGetClientIp

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetClientIp(const char** ppchClientIp);

```

Retrieves the IPv4-based IP for the client system associated with the current streaming session.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchClientIp | Output IPv4 in string format. Example: "192.168.0.1". Call gfnFree to release the memory when done. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnNoData | No IP data found. |
| gfnCallWrongEnvironment | Function called in a client environment. |

### gfnGetClientLanguageCode

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetClientLanguageCode(const char** ppchLanguageCode);

```

Gets user's client language code in the form `"<lang>-<country>"` using a standard RFC3066 language locale code (example: en_US) and ISO 3166-1 Alpha-2 country code (example: GB).

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchLanguageCode | Language code as a string. Example: "en_US-GB". Call gfnFree to release the memory when done. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |

### gfnGetClientCountryCode

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetClientCountryCode(char* pchCountryCode, unsigned int length);

```

Gets user’s client country code using ISO 3166-1 Alpha-2 country code.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchCountryCode | Country code as a 2 character string. Example: "US". |
| length | Length of pchCountryCode character array, the length should be at least 3. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |


### gfnGetClientInfo

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnGetClientInfo(GfnClientInfo* clientInfo);

```

Gets various information about the local client system and environment.

Environment: Cloud   
Platform: Windows, Linux     
Parameters:   
| Value | Meaning |
| :---- | :---- |
| clientInfo | Pointer to a GfnClientInfo struct. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |



### gfnGetSessionInfo

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnGetSessionInfo(GfnSessionInfo* sessionInfo);

```

Gets various information about the current streaming session.

Environment: Cloud   
Platform: Windows, Linux     
Parameters:   
| Value | Meaning |
| :---- | :---- |
| sessionInfo | Pointer to a GfnSessionInfo struct. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |


### gfnGetPartnerData

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetPartnerData(const char** ppchPartnerData);

```

Gets various information about the current streaming session.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchPartnerData | Populated with the partner data in string form if found. Call gfnFree to release the memory when done with data. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnNoData | No partner data found in session data. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |

### gfnGetPartnerSecureData

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnGetPartnerSecureData(const char** ppchPartnerSecureData);

```

Retrieves secure partner data that is sent to the session as part of the response to Deep Link nonce validation.

Environment: Cloud   
Platform: Windows, Linux     
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchPartnerSecureData | Populated with the secure partner data in string form if found. Call gfnFree to release the memory when done. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnNoData | No secure partner data found in session data. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |



### Action Request Functions
The functions in this section are used by titles to request a type of action be performed in the GFN session or on the client device.

### gfnFree

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnFree(const char** ppchData);

```

Releases memory allocated by Get functions such as, but not limited to, gfnGetPartnerData, gfnGetPartnerSecureData, or gfnGetTitlesAvailable.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| ppchData | Pointer to allocated string memory to be freed. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | Function called in a client environment. |


### gfnSetEditBoxRegion

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnSetEditBoxRegion(unsigned int id, GfnRect* region, unsigned int flags);

```

Sends Edit-Box region coordinates and flags to GFN Client.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| id | Unique unsigned int type identifier for this EditBox region. |
| region | Region coordinates of the EditBox. |
| flags | Bitflag for EditBox region flags. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInputExpected | Expected zone to have a value. |
| gfnComError |There was SDK internal communication error. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnThrottled | API call was throttled for exceeding limit. |
| gfnUnhandledException | API ran into an unhandled error and caught an exception before it returned to client code. |

### gfnClearEditBoxRegion

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnClearEditBoxRegion(unsigned int id);

```

Clears the EditBox region specified using unique id.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| id | Unique id of EditBox region that is to be cleared. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInputExpected | Expected zone to have a value. |
| gfnComError |There was SDK internal communication error. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnThrottled | API call was throttled for exceeding limit. |
| gfnUnhandledException | API ran into an unhandled error and caught an exception before it returned to client code. |


### gfnSetActionZone

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnSetActionZone(GfnActionType type, unsigned int id, GfnRect* zone);

```

Defines active zone coordinates for GFN to interact with.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| type | Populated with relevant GfnActionType. |
| id | Unique unsigned int type identifier for this action zone. |
| zone | To enable action zone set this parameter to GfnRect coordinates of the zone, to disable action zone set this parameter to NULL. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnInputExpected | Expected zone to have a value. |
| gfnComError |There was SDK internal communication error. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnThrottled | API call was throttled for exceeding limit. |
| gfnUnhandledException | API ran into an unhandled error and caught an exception before it returned to client code. |


### gfnSendMessage

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnSendMessage(const char* pchMessage, unsigned int length);

```

Sends a custom message communication from the app to the client. This message is "fire-and-forget", and does not wait for the message to be delivered to return status. Latency is best effort and not guaranteed.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchMessage | Character string of the message to send. |
| id | Unique unsigned int type identifier for this action zone. |
| length | Length of pchMessage in characters, which cannot exceed 8K in length. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnComError |There was SDK internal communication error. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnThrottled | API call was throttled for exceeding limit. |
| gfnUnhandledException | API ran into an unhandled error and caught an exception before it returned to client code. |



### gfnOpenURLOnClient

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnOpenURLOnClient(const char* pchUrl);

```

Requests the client application to open a URL in their local web browser.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchUrl | A URL which should be opened on the client's local browser. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnComError |There was SDK internal communication error. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnThrottled | API call was throttled for exceeding limit. |
| gfnUnhandledException | API ran into an unhandled error and caught an exception before it returned to client code. |



### gfnSetAppState

```c

NVGFNSDK_EXPORT GfnRuntimeError gfnSetAppState(GfnAppState appState);

```

Allows the application or game to inform GFN of its overall running state.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| appState | A GfnAppState value. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnAPINotInit | SDK was not initialized. |
| gfnAPINotFound | The API was not found in the GeForce NOW SDK Library. |
| gfnInvalidParameter | NULL pointer passed in. |


### gfnAppReady

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnAppReady(bool success, const char * status);

```

Notifies GFN that an application is ready to be displayed.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| success | True if startup has succeeded, false if not. |
| status | Optional status of startup operations. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnCallWrongEnvironment | If called in a client environment. |



### gfnSetupTitle

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnSetupTitle(const char* pchPlatformAppId);

```

Requests GeForce NOW to set up a GFN title to be readied for launch.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchPlatformAppId | Identifier of the requested application to setup. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnAPINotInit | SDK was not initialized. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | If called in a client environment. |
| gfnSetupTitleFailure | The GeForce NOW SDK failed to set up the title. |

### gfnTitleExited

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnTitleExited(const char* pchPlatformId, const char* pchPlatformAppId);

```

Notifies GFN that an application has exited. GFN will then start the shutdown process for that application.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pchPlatformId | Identifier of the launcher service, e.g. "Steam". |
| pchPlatformAppId | Identifier of the application that has exited. |


Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, otherwise, appropriate error code. |
| gfnAPINotInit | SDK was not initialized. |
| gfnInvalidParameter | NULL pointer passed in. |
| gfnCallWrongEnvironment | If called in a client environment. |


### Callback Functions
The functions in this section of the document provide the ability to receive notifications related to changes in session state or data via C++-style callbacks. Use these functions to register for a specific callback type.

### gfnRegisterExitCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterExitCallback(ExitCallbackSig exitCallback, void* pUserContext);

```

Register an application function with GeForce NOW that is called when the user has requested to end the GeForce NOW session to allow the application to perform exit operations during session shutdown.

Environment: Cloud   
Platform: Windows, Linux     
Parameters:   
| Value | Meaning |
| :---- | :---- |
| exitCallback | Function pointer to application code to call when GeForce NOW needs to exit the game. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |

### gfnRegisterPauseCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterPauseCallback(PauseCallbackSig pauseCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when GeForce NOW needs to pause the game on the user's behalf. For Multiplayer games, it is recommended that this is implemented similar to a client disconnect.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| pauseCallback | Function pointer to application code to call when GeForce NOW needs to pause game. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |

### gfnRegisterInstallCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterInstallCallback(InstallCallbackSig installCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called after a successful call to gfnSetupTitle. Typically, the callback would handle any additional installation steps that are necessary after GeForce NOW performs its own setup for a given title.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:  
| Value | Meaning |
| :---- | :---- |
| installCallback | Function pointer to application code to call after GeForce NOW successfully performs its own title setup. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |

### gfnRegisterSaveCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterSaveCallback(SaveCallbackSig saveCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when GeForce NOW needs the application to save user progress. It is recommended that this be implemented as an autosave if such a feature is supported by your application.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| saveCallback | Function pointer to application code to call when GeForce NOW needs the application to save user data. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |

### gfnRegisterSessionInitCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterSessionInitCallback(SessionInitCallbackSig sessionInitCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when a GeForce NOW user has connected to the game seat to start a streaming session. This callback is used as part of the Pre-warm feature. 

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| sessionInitCallback | Function pointer to application code to call when GFN is ready for the application to finalize loading. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |


### gfnRegisterMessageCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterMessageCallback(MessageCallbackSig messageCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when certain client info that is part of gfnGetClientInfo API changes. 

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| messageCallback | Function pointer to application code to call when a message has been received. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |



### gfnRegisterClientInfoCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterClientInfoCallback(ClientInfoCallbackSig clientInfoCallback, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when certain client info that is part of gfnGetClientInfo API changes.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| clientInfoCallback | Function pointer to application code to call when client information has changed. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |



### gfnRegisterNetworkStatusCallback

```c

NVGFNSDK_EXPORT GfnRuntimeError NVGFNSDKApi gfnRegisterNetworkStatusCallback(NetworkStatusCallbackSig networkStatusCallback, unsigned int updateRateMs, void* pUserContext);

```

Register an application callback with GeForce NOW to be called when client latency changes.

Environment: Cloud   
Platform: Windows, Linux   
Parameters:   
| Value | Meaning |
| :---- | :---- |
| networkStatusCallback | Function pointer to application code to call when network latency has changed. |
| updateRateMs | The targeted rate at which callbacks should occur. |
| pUserContext | Pointer to user context, which will be passed unmodified to the registered callback. Can be NULL. |

Return Values:   
| Value | Meaning |
| :---- | :---- |
| gfnSuccess | On success, when running in a GeForce NOW environment |
| gfnInternalError | If callback could not be registered |
| gfnInvalidParameter | If callback function pointer is NULL |
| gfnCallWrongEnvironment | If callback could not be registered since this function was called outside of a cloud execution environment |
| gfnAPINotInit | SDK was not initialized |
| gfnAPINotFound | If the API was not found in the GeForce NOW SDK Library |


## Data Types
This section of the document provides definitions of the various data types used by GFN SDK functions.

### GfnApplicationCallbackResult

```c

typedef enum GfnApplicationCallbackResult
{
	crCallbackSuccess =  0, // Return to indicate that a callback has performed the requested operation
	crCallbackFailure = -1  // Return to indicate that a callback did not perform the requested operation
} GfnApplicationCallbackResult;

```

Returned by callbacks the application registers with the GeForce NOW Runtime SDK, or passes in to asynchronous SDK calls.

### GfnIsRunningInCloudAssurance

```c

typedef enum GfnIsRunningInCloudAssurance
{
	gfnNotCloud = 0,                  // Not considered to be running in GFN cloud, as it looks like a client/local system.
	gfnIsCloudLowAssurance = 1,       // Considered to be running in GFN Cloud, using software heuristics that are not guaranteed against circumvention.
	gfnIsCloudMidAssurance = 2,       // Considered to be running in GFN Cloud, using software and network heuristics that are difficult to circumvent.
	gfnIsCloudHighAssurance = 3       // Considered to be running in GFN Cloud, using software and hardware heuristics that are near impossible to circumvent.
} GfnIsRunningInCloudAssurance;

```

Returned from gfnIsRunningInCloudSecure to denote security assurance that the calling application is running, untampered, in the GFN cloud environment.


### GfnCloudCheckChallenge

```c

typedef struct GfnCloudCheckChallenge
{
    const char* nonce;      // A pointer to randomly generated data to be used as the payload of the challenge message. If set to nullptr GFN SDK will generate a random challenge on its own.
    unsigned int nonceSize; // The size of the data pointed to by nonce. It is recommended that nonce is at least 16 byte (128 bit) long.
} GfnCloudCheckChallenge;

```

Challenge data passed in to gfnCloudCheck API

### GfnCloudCheckResponse

```c
typedef struct GfnCloudCheckResponse
{
    const char* attestationData;        // If the cloud check is successful, the attestationData is cryptographically signed message containing the same nonce that was provided in the challenge; otherwise, this field is set to nullptr.
    unsigned int attestationDataSize;   // The size of the attestationData field in case it is not empty; zero otherwise
} GfnCloudCheckResponse;

```

Response data received in the gfnCloudCheck API.



### GfnCloudType

```c

typedef enum GfnCloudType
{
    CC_CLOUD_TYPE_NULL = 0x00000000,      // Unknown session type
    CC_CLOUD_TYPE_TRUSTED = 0x00000001,   // Trusted session type
    CC_CLOUD_TYPE_OPEN = 0x00000002,      // Open session type
    CC_CLOUD_TYPE_ANY = 0x00000003        // Other session type
} GfnCloudType;

```

Type of GFN Cloud environment as returned from the gfnCloudType function.


### TitleInstallationInformation

```c

typedef struct TitleInstallationInformation
{
	const char* pchPlatformAppId; // Platform application ID passed into the API.
	const char* pchBuildPath;     // The path at which game build files can be found.
	const char* pchMetadataPath;  // Optionally contains the path at which game metadata can be found, else NULL.
} TitleInstallationInformation;

```

Input to the function registered via gfnRegisterInstallCallback (if any).

### GfnActionType

```c

typedef enum GfnActionType
{
	gfnActionNone = 0,    // No event
	gfnEditBox = 1,       // Action to specify an editable text box rect on screen
	gfnActionMAX          // Sentinel value, do not use
} GfnActionType;

```

Specifies the action in GfnSetActionZone API. Deprecated with SetActionZone removal.


### GfnOsType

```c

typedef enum GfnOsType
{
	gfnUnknownOs = 0,       // Unknown OS
	gfnWindows = 1,         // Windows 
	gfnMacOs = 2,           // MAC
	gfnShield = 3,          // Nvidia Shield
	gfnAndroid = 4,         // Android
	gfnIOs = 5,             // IOS
	gfnIPadOs = 6,          // IPadOS
	gfnChromeOs = 7,        // ChromeOS
	gfnLinux = 8,           // Linux
	gfnTizen = 9,           // Tizen
	gfnWebOs = 10,          // WebOS
	gfnTvOs = 11,           // TVOS
	gfnOsTypeMax = 11       // OSTypeMax
} GfnOsType;

```

Types of operating systems that can be reported by the SDK.

### GfnInputType

```c

typedef enum GfnInputType
{
	gfnUnknownInput = 0,
	gfnMouse = 1,
	gfnKeyboard = 2,
	gfnMouseKeyboard = 3,
	gfnXInputGamepad = 4,
	gfnXInputGamepadPartial = 5,
	gfnDirectInputGamepad = 6,
	gfnDirectInputGamepadPartial = 7,
	gfnJoystick = 8,
	gfnWheel = 9,
	gfnTouchscreen = 10,
	gfnWiiController = 11,
	gfnKinectController = 12,
	gfnInputTypeMax = 12
} GfnInputType;

```

Types of input used by the client device running the GFN client application.

### GfnClientInfo

```c

typedef struct
{
	unsigned int version;               // Deprecated, value will be ignored
	GfnOsType osType;                   // Operating System type
	char ipV4[IP_V4_SIZE];              // IPV4 address, example - "192.168.0.1"
	char country[CC_SIZE];              // ISO 3166-1 alpha-2 standard country code, example - "us"
	char locale[LOCALE_SIZE];           // ISO 639-1 Alpha-2 standard locale, example - "en-us"
	unsigned int RTDAverageLatencyMs;   // Round Trip Delay - Average Latency in milliseconds
	GfnResolutionInfo clientResolution; // Client device's physical resolution, if reported. If client does not report, expect values of zero.
} GfnClientInfo;

```

Data structure for client device information.

### GfnClientDataChangeType

```c

typedef enum GfnClientInfoChangeType
{
	gfnOs = 0,                          // Change in OS of GFN Client
	gfnIP = 1,                          // Change in IP of GFN Client (due to device switch/reconnect)
	gfnClientResolution = 2,            // Change in resolution of GFN Client
	gfnSafeZone = 3,                    // Change in safe zone rectangle (due to device switch or rotation)
	gfnClientDataChangeTypeMax = 3      // Sentinel value for GfnClientInfoChangeType
	gfnClientDataChangeTypeMax = 0      // Sentinel value for GfnClientInfoChangeType
} GfnClientDataChangeType;

```

Data structure used by the ClientInfo callback to notify changes to client device information, for example, when a GFN session resumes on a different device.

### GfnClientInfoUpdateData

```c

typedef struct GfnClientInfoUpdateData
{
	unsigned int version;               // Deprecated, value will be ignored
	GfnClientDataChangeType updateType; // Type of GFN Client data that changed
	union
	{
		GfnOsType osType;               // Operating System type
		char ipV4[IP_V4_SIZE];          // IPV4 address, example - "192.168.0.1"
		GfnResolutionInfo clientResolution; // Client device's physical resolution, if reported. If client does not report, expect values of zero.
		GfnRect safeZone;                   // Client safe zone rectangle (title-safe area), in normalized coordinates. If the entire screen is title-safe because of device type, then expect values of zero.
	} data;
} GfnClientInfoUpdateData;

```

An update notification used by the ClientInfo callback for a piece of client info data that changed. Used in conjunction with GfnClientInfoChangeType.






### GfnNetworkStatusChangeType

```c

typedef enum GfnNetworkStatusChangeType
{
    gfnRTDAverageLatency          = 0, // Change in RTDAverageLatency
    gfnNetworkStatusChangeTypeMax = 0  // Sentinel value for GfnNetworkStatusChangeType
} GfnNetworkStatusChangeType;

```

An update notification for a piece of network status data.

### GfnNetworkStatusChangeType

```c

typedef struct GfnNetworkStatusUpdateData
{
    unsigned int version;                   // Deprecated, value will be ignored
    GfnNetworkStatusChangeType updateType;  // Network Status Update type
    union
    {
        unsigned int RTDAverageLatencyMs;   // Round Trip Delay - Average Latency in milliseconds
    } data;
} GfnNetworkStatusChangeType;

```

Types of input used by the client device running the GFN client application.



### GfnSessionInfo

```c

typedef struct
{
	unsigned int sessionMaxDurationSec;      // Maximum total time allowed for the session in seconds
	unsigned int sessionTimeRemainingSec;    // Nominal time remaining in the session in seconds
	char sessionId[SESSION_ID_SIZE];         // NVIDIA-defined unique identifier for the session
	bool sessionRTXEnabled;                  // Defines if RTX support is enabled for the session
} GfnSessionInfo;

```

Data structure used by the gfnGetSessionInfo function.
 


### GfnAppState

```c

typedef enum GfnAppState
{
	gfnAppLoading = 0,              // Application is loading
	gfnAppInMenu = 1,               // Application has opened menu options for the user
	gfnAppRunning = 2,              // Application is running and accepting user input
	gfnAppPaused = 3,               // Application is paused, no input accepted
	gfnAppExiting = 4,              // Application is exiting
	gfnAppErrorDetected = 5,        // Application has run into a critical error
	gfnAppStateMax = 6              // Sentinel value, do not use
} GfnAppState;

```

Application/game state data passed in to gfnSetAppState function.


### GfnRectFormat

```c

typedef enum GfnRectFormat
{
    // value1 : Left
    // value2 : Top
    // value3 : Right
    // value4 : Bottom
    gfnRectLTRB = 0,

    // value1 : Top Left Corner's X coordinate
    // value2 : Top Left Corner's Y coordinate
    // value3 : Width
    // value4 : Height
    gfnRectXYWH,

    // Sentinel value, do not use
    gfnRectMAX
} GfnRectFormat;

```

Formats to specify a rectangle with top-left as origin.

### GfnRect

```c

typedef struct GfnRect
{
    float value1;         // value1 as per format
    float value2;         // value2 as per format
    float value3;         // value3 as per format
    float value4;         // value4 as per format
    bool normalized;      // true : coordinates are normalized between 0.0-1.0, false : absolute coordinates
    GfnRectFormat format; // rect format as listed in GfnRectFormat
} GfnRect;

```

Struct to reference a rectangle.

### GfnString

```c

typedef struct GfnString
{
    const char* pchString; // char pointer to string
    unsigned int length;   // length of string
} GfnString;

```

Struct to reference a string with length data.

_V3
### GfnResolutionInfo

```c

typedef struct GfnResolutionInfo
{
	unsigned int verticalPixels; // height of the resolution in pixels
	unsigned int horizontalPixels; // width of the resolution in pixels
} GfnResolutionInfo;

```

Data structure used to represent the display resolution of the client device used for the current GFN session.
_V3



## Function Return/Error Values
This section of the document provides the list of all possible API function return values and their meanings as defined by the *GfnError* data type.

| Value | Meaning |
| :---- | :---- |
| gfnSuccess | Function Success. |
| gfnInitSuccessClientOnly | SDK initialized, but only cloud independent functionality available. |
| gfnInitSuccessCloudOnly | SDK initialized, but only cloud functionality is available. |
| gfnInitFailure | SDK initialization failure for any reason other than memory allocation failure. |
| gfnDllNotPresent | GFN-related DLL is not present. |
| gfnComError | GFN SDK internal component communication error. |
| gfnLibraryCallFailure |GFN SDK could not serve the request. |
| gfnIncompatibleVersion | Incompatible version of GFN SDK found. |
| gfnUnableToAllocateMemory | Unable to allocate memory. |
| gfnInvalidParameter | Invalid parameter. |
| gfnInternalError | Generic GFN SDK internal error. |
| gfnUnsupportedAPICall | API Call is not supported. |
| gfnInvalidToken |  Invalid toke. |
| gfnTimedOut | Operation timed out. |
| gfnSetupTitleFailure | Failed to setup title. |
| gfnClientDownloadFailed | GFN SDK component download failure. |
| gfnCallWrongEnvironment | Function limited to specific environment (e.g., Cloud) and called in wrong environment (e.g., Client). |
| gfnWebApiFailed | A call to a NVIDIA Web API failed to return valid data. |
| gfnStreamFailure | GeForce NOW Streamer hit a failure while starting a stream. |
| gfnAPINotFound | Library API call not found. |
| gfnAPINotInit | API not initialized. |
| gfnStreamStopFailure | Failed to stop active streaming session. |
| gfnUnhandledException | Unhandled exception inside GFN SDK API call. |
| gfnIPCFailure | Failure in GFN SDK IPC. |
| gfnCanceled | Activity was canceled by user. |
| gfnElevationRequired | API call required to be run from an elevated process. |
| gfnThrottled | API call throttled due to too many or too frequent calls. |
| gfnInputExpected | API call was expecting input param to have a value. |
| gfnBinarySignatureInvalid | An attempt to load a binary failed because the digital signature was found to be invalid. |
| gfnCloudLibraryNotFound | Necessary GFN cloud-based SDK library cannot be found. |
| gfnClientLibraryNotFound | Necessary GFN client-based SDK library cannot be found (Deprecated). |
| gfnNoData | Requested data is empty or doesn't exist. |
| gfnNotAuthorized | API Call failed because it was called from unauthorized process. |
| gfnBackendError | Failed to communicate with the GFN backend service. |
| gfnEntitlementNotFound | User/App is not entitled. |

