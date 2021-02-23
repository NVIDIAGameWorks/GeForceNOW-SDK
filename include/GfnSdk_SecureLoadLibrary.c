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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Shlobj.h>
#include <WinTrust.h>
#include <wchar.h> // Needed for conversion to WCHAR for Win32 APIs that require it
#include <windows.h>
#include <strsafe.h>

#include "GfnSdk_SecureLoadLibrary.h"

// Internal function forward declarations
static BOOL gfnInternalFileExists(LPCWSTR szFileName);
static BOOL gfnInternalContainsAbsolutePath(LPCWSTR szFileName);
static HANDLE gfnInternalLockFileForGenericReadAccess(LPCWSTR szFilePath);
static LPWSTR gfnInternalCreateUnicodeStringFromAscii(LPCSTR szAscii);

typedef enum tagSignatureType
{
    SignatureTypeAny,
    SignatureTypeGfn,
    SignatureTypeNvidia
} SignatureType;
static BOOL gfnInternalVerifyFileSignature(LPCWSTR fileName, SignatureType signatureType);
static BOOL gfnInternalVerifyFileSignatureInfo(
    PCMSG_SIGNER_INFO pSignerInfo,
    HCERTSTORE hStore,
    LPCWSTR fileName,
    DWORD index,
    SignatureType signatureType);
static HMODULE gfnInternalSecureLoadLibraryW(LPCWSTR filePath, DWORD dwFlags, SignatureType signatureType);
static HMODULE gfnInternalSecureLoadLibraryA(LPCSTR filePath, DWORD dwFlags, SignatureType signatureType);

// Just emphasising that LocalFree ignores NULL args:
#define SafeLocalFree(x) LocalFree(x)
// Close HANDLE without changing last OS error:
#define SafeCloseHandle(x)                                \
    if ((NULL != (x)) && (INVALID_HANDLE_VALUE != (x)))   \
    {                                                     \
        DWORD lastError = GetLastError();                 \
        CloseHandle(x); (x) = INVALID_HANDLE_VALUE;       \
        SetLastError(lastError);                          \
    }

// ===================================================================
// Public API functions are defined in this section
// ===================================================================
HMODULE gfnSecureLoadClientLibraryW(LPCWSTR filePath, DWORD dwFlags)
{
    return gfnInternalSecureLoadLibraryW(filePath, dwFlags, SignatureTypeNvidia);
}

HMODULE gfnSecureLoadClientLibraryA(LPCSTR filePath, DWORD dwFlags)
{
    return gfnInternalSecureLoadLibraryA(filePath, dwFlags, SignatureTypeNvidia);
}

HMODULE gfnSecureLoadCloudLibraryW(LPCWSTR filePath, DWORD dwFlags)
{
    return gfnInternalSecureLoadLibraryW(filePath, dwFlags, SignatureTypeGfn);
}

HMODULE gfnSecureLoadCloudLibraryA(LPCSTR filePath, DWORD dwFlags)
{
    return gfnInternalSecureLoadLibraryA(filePath, dwFlags, SignatureTypeGfn);
}

// ===================================================================
// Internal functions defined below. Do not use directly.
// ===================================================================

static HMODULE gfnInternalSecureLoadLibraryW(LPCWSTR filePath, DWORD dwFlags, SignatureType signatureType)
{
    HANDLE hFileLock = INVALID_HANDLE_VALUE;
    HMODULE pResult = NULL;

    SetLastError(ERROR_SUCCESS);

    // filePath must specifiy an absolute path.
    if (!gfnInternalContainsAbsolutePath(filePath))
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
    }
    else if (!gfnInternalFileExists(filePath))
    {
        SetLastError(ERROR_MOD_NOT_FOUND);
    }
    else if (INVALID_HANDLE_VALUE == (hFileLock = gfnInternalLockFileForGenericReadAccess(filePath)))
    {
        SetLastError(ERROR_SHARING_VIOLATION);
    }
    else
    {
        BOOL bSignatureVerified = gfnInternalVerifyFileSignature(filePath, signatureType);
        if (!bSignatureVerified)
        {
            SetLastError((DWORD)CRYPT_E_NO_MATCH);
        }
        else
        {
            pResult = LoadLibraryExW(filePath, NULL, dwFlags);
        }

        SafeCloseHandle(hFileLock);
    }

    return pResult;
}

static HMODULE gfnInternalSecureLoadLibraryA(LPCSTR filePath, DWORD dwFlags, SignatureType signatureType)
{
    // Defering to UNICODE version to reduce complexity/redundancy
    HMODULE hResult = NULL;
    LPWSTR unicodeFilePath = NULL;
    if (!filePath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    unicodeFilePath = gfnInternalCreateUnicodeStringFromAscii(filePath);
    if (unicodeFilePath)
    {
        hResult = gfnInternalSecureLoadLibraryW(unicodeFilePath, dwFlags, signatureType);
    }
    SafeLocalFree(unicodeFilePath);
    return hResult;
}

#pragma warning(disable: 4706) // Don't warn about assignments within conditional expressions

static HMODULE hModCrypt32 = NULL;
typedef BOOL(WINAPI* PfnCryptMsgClose)(IN HCRYPTMSG hCryptMsg);
static PfnCryptMsgClose pfnCryptMsgClose = NULL;

typedef BOOL(WINAPI* PfnCertCloseStore)(IN HCERTSTORE hCertStore, DWORD dwFlags);
static PfnCertCloseStore pfnCertCloseStore = NULL;

typedef BOOL(WINAPI* PfnCertFreeCertificateContext)(IN PCCERT_CONTEXT pCertContext);
static PfnCertFreeCertificateContext pfnCertFreeCertificateContext = NULL;

typedef PCCERT_CONTEXT(WINAPI* PfnCertFindCertificateInStore)(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void* pvFindPara,
    IN PCCERT_CONTEXT pPrevCertContext
    );
static PfnCertFindCertificateInStore pfnCertFindCertificateInStore = NULL;

typedef BOOL(WINAPI* PfnCryptMsgGetParam)(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void* pvData,
    IN OUT DWORD* pcbData
    );
static PfnCryptMsgGetParam pfnCryptMsgGetParam = NULL;

typedef BOOL(WINAPI* PfnCryptQueryObject)(
    DWORD            dwObjectType,
    const void* pvObject,
    DWORD            dwExpectedContentTypeFlags,
    DWORD            dwExpectedFormatTypeFlags,
    DWORD            dwFlags,
    DWORD* pdwMsgAndCertEncodingType,
    DWORD* pdwContentType,
    DWORD* pdwFormatType,
    HCERTSTORE* phCertStore,
    HCRYPTMSG* phMsg,
    const void** ppvContext
    );
static PfnCryptQueryObject pfnCryptQueryObject = NULL;

typedef BOOL(WINAPI* PfnCryptDecodeObjectEx)(
    IN DWORD              dwCertEncodingType,
    IN LPCSTR             lpszStructType,
    IN const BYTE* pbEncoded,
    IN DWORD              cbEncoded,
    IN DWORD              dwFlags,
    IN PCRYPT_DECODE_PARA pDecodePara,
    OUT void* pvStructInfo,
    IN OUT DWORD* pcbStructInfo
    );
static PfnCryptDecodeObjectEx pfnCryptDecodeObjectEx = NULL;

typedef PCCERT_CONTEXT(WINAPI* PfnCertGetIssuerCertificateFromStore)(
    IN          HCERTSTORE     hCertStore,
    IN          PCCERT_CONTEXT pSubjectContext,
    IN OPTIONAL PCCERT_CONTEXT pPrevIssuerContext,
    IN OUT      DWORD* pdwFlags
    );
static PfnCertGetIssuerCertificateFromStore pfnCertGetIssuerCertificateFromStore = NULL;

typedef DWORD(WINAPI* PfnCertGetNameStringA)(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwType,
    IN DWORD dwFlags,
    IN void* pvTypePara,
    OUT OPTIONAL LPSTR pszNameString,
    IN DWORD cchNameString
    );
static PfnCertGetNameStringA pfnCertGetNameStringA = NULL;

typedef LONG(WINAPI* PfnWinVerifyTrust)(IN HWND hWnd, IN GUID* pgActionID, IN LPVOID pWVTData);
static PfnWinVerifyTrust pfnWinVerifyTrust = NULL;

typedef LONG(APIENTRY* PfnRegOpenKeyExW)(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    );
static PfnRegOpenKeyExW pfnRegOpenKeyExW = NULL;

typedef LONG(APIENTRY* PfnRegQueryValueExW)(
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN  LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );
static PfnRegQueryValueExW pfnRegQueryValueExW = NULL;

typedef LONG(APIENTRY* PfnRegCloseKey)(IN HKEY hKey);
static PfnRegCloseKey pfnRegCloseKey = NULL;

typedef HRESULT(WINAPI* PfnSHGetFolderPath_W)(HWND, int, HANDLE, DWORD, LPWSTR);
static PfnSHGetFolderPath_W pfnSHGetFolderPath = NULL;

typedef SC_HANDLE(WINAPI* PfnOpenSCManagerW)(
    IN LPCWSTR lpMachineName,
    IN LPCWSTR lpDatabaseName,
    IN DWORD dwDesiredAccess
    );
static PfnOpenSCManagerW pfnOpenSCManagerW = NULL;

typedef SC_HANDLE(WINAPI* PfnOpenServiceW)(
    IN SC_HANDLE hSCManager,
    IN LPCWSTR lpServiceName,
    IN DWORD dwDesiredAccess
    );
static PfnOpenServiceW pfnOpenServiceW = NULL;

typedef BOOL(WINAPI* PfnQueryServiceStatus)(
    IN SC_HANDLE hService,
    OUT LPSERVICE_STATUS lpServiceStatus
    );
static PfnQueryServiceStatus pfnQueryServiceStatus = NULL;

typedef BOOL(WINAPI* PfnCloseServiceHandle)(
    IN SC_HANDLE hSCObject
    );
static PfnCloseServiceHandle pfnCloseServiceHandle = NULL;

typedef HCRYPTMSG(WINAPI* PfnCryptMsgOpenToDecode)(
    IN DWORD             dwMsgEncodingType,
    IN DWORD             dwFlags,
    IN DWORD             dwMsgType,
    IN HCRYPTPROV_LEGACY hCryptProv,
    IN PCERT_INFO        pRecipientInfo,
    IN PCMSG_STREAM_INFO pStreamInfo
);
static PfnCryptMsgOpenToDecode pfnCryptMsgOpenToDecode = NULL;

typedef BOOL(WINAPI* PfnCryptMsgUpdate)(
    IN HCRYPTMSG  hCryptMsg,
    IN const BYTE* pbData,
    IN DWORD      cbData,
    IN BOOL       fFinal
);
static PfnCryptMsgUpdate pfnCryptMsgUpdate = NULL;

typedef HCERTSTORE(WINAPI* PfnCertOpenStore)(
    IN LPCSTR            lpszStoreProvider,
    IN DWORD             dwEncodingType,
    IN HCRYPTPROV_LEGACY hCryptProv,
    IN DWORD             dwFlags,
    IN const void* pvPara
);
static PfnCertOpenStore pfnCertOpenStore = NULL;

typedef BOOL(WINAPI* PfnCertGetCertificateChain)(
    IN HCERTCHAINENGINE       hChainEngine,
    IN PCCERT_CONTEXT         pCertContext,
    IN LPFILETIME             pTime,
    IN HCERTSTORE             hAdditionalStore,
    IN PCERT_CHAIN_PARA       pChainPara,
    IN DWORD                  dwFlags,
    IN LPVOID                 pvReserved,
    OUT PCCERT_CHAIN_CONTEXT* ppChainContext
);
PfnCertGetCertificateChain pfnCertGetCertificateChain = NULL;
typedef VOID(WINAPI* PfnCertFreeCertificateChain)(
    IN PCCERT_CHAIN_CONTEXT pChainContext
);
static PfnCertFreeCertificateChain pfnCertFreeCertificateChain = NULL;

static BOOL gfnInternalFileExists(LPCWSTR szFileName)
{
    DWORD fileAttributes = GetFileAttributesW(szFileName);
    const DWORD nonFileAttributes = FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY;
    return ((INVALID_FILE_ATTRIBUTES == fileAttributes) || (fileAttributes & nonFileAttributes)) ? FALSE : TRUE;
}

static BOOL gfnInternalContainsAbsolutePath(LPCWSTR szFileName)
{
    if (!szFileName)
    {
        return FALSE;
    }

    if ((szFileName[0] == '\\') || (szFileName[0] == '/'))
    {
        return TRUE;
    }

    if ((isalpha(szFileName[0]) && (szFileName[1] == ':')) && ((szFileName[2] == '\\') || (szFileName[2] == '/')))
    {
        return TRUE;
    }

    return FALSE;
}

static HANDLE gfnInternalLockFileForGenericReadAccess(LPCWSTR szFilePath)
{
    return CreateFileW(
        szFilePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
}

static LPWSTR gfnInternalCreateUnicodeStringFromAscii(LPCSTR szAscii)
{
    LPWSTR pResult = NULL;
    UINT i = 0;

    if (!!szAscii)
    {
        if (pResult = (LPWSTR)LocalAlloc(LPTR, (strlen(szAscii) + 1) * sizeof(WCHAR)))
        {
            for (i = 0; i < strlen(szAscii); ++i)
            {
                pResult[i] = szAscii[i];
            }
        }
    }
    return pResult;
}

LPWSTR gfnInternalCreateSystemFilePath(LPCWSTR szFileName)
{
    LPWSTR pResult = NULL;
    UINT i;

    if (!!szFileName)
    {
        DWORD dwLength = GetSystemDirectoryW(pResult, 0);
        pResult = (LPWSTR)LocalAlloc(LPTR, (dwLength + 1 + wcslen(szFileName)) * sizeof(WCHAR));
        if (!pResult)
        {
            return NULL;
        }

        dwLength = GetSystemDirectoryW(pResult, dwLength);
        if (pResult[dwLength - 1] != '\\')
        {
            pResult[dwLength++] = '\\';
        }

        for (i = 0; i < wcslen(szFileName); ++i)
        {
            pResult[dwLength + i] = szFileName[i];
        }
    }
    return pResult;
}

typedef enum
{
    eWindowsVistaBuild = 6000,
    eWindows7Build = 7600,
    eWindows8Build = 9200,
    eWindows8Point1Build = 9600,
    eWindows10TH1Build = 10240,
    eWindows10TH2Build = 10586,
    eWindows10RS1DriverStoreSwitch = 14308,
    eWindows10RS4Build = 17130
}
KnownWindowsBuildNumbers;

#define VALID_LOADLIBRARYEX_FLAGS ~(LOAD_LIBRARY_SEARCH_APPLICATION_DIR |\
                                    LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |\
                                    LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |\
                                    LOAD_LIBRARY_SEARCH_USER_DIRS |\
                                    LOAD_WITH_ALTERED_SEARCH_PATH)

static HMODULE gfnInternalLoadSystemLibrary(const LPCWSTR fileName, DWORD dwFlags)
{
    LPWSTR systemFilePath = NULL;
    HMODULE hRet;
    static DWORD dwBuildNumber = 0;
    dwFlags &= VALID_LOADLIBRARYEX_FLAGS;

    if (dwBuildNumber == 0)
    {
        OSVERSIONINFOEXW osvi;
        DWORDLONG dwlConditionMask = VerSetConditionMask(0, VER_BUILDNUMBER, VER_GREATER_EQUAL);
        memset(&osvi, 0, sizeof(osvi));
        osvi.dwBuildNumber = eWindows8Build;

        dwBuildNumber = VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, dwlConditionMask) ? eWindows8Build : eWindows7Build;
    }

    if (dwBuildNumber >= eWindows8Build)
    {
        dwFlags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
    }
    else // Win7
    {
        dwFlags &= ~LOAD_LIBRARY_SEARCH_SYSTEM32;
        systemFilePath = gfnInternalCreateSystemFilePath(fileName);
    }
    hRet = LoadLibraryExW(systemFilePath ? systemFilePath : fileName, NULL, dwFlags);
    SafeLocalFree(systemFilePath);
    return hRet;
}

static BOOL gfnInternalCheckCert(
    const PCCERT_CONTEXT pCert,
    BOOL bRootCheck,
    LPCSTR name,
    size_t publicKeySize,
    const BYTE* publicKey)
{
    if (name != NULL)
    {
        char szCertName[256] = "";
        pfnCertGetNameStringA(
            pCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            bRootCheck ? CERT_NAME_ISSUER_FLAG : 0,
            NULL,
            szCertName,
            sizeof(szCertName));

        if (strcmp(szCertName, name) != 0)
        {
            return FALSE;
        }
    }

    if (publicKeySize > 0 && publicKey != NULL)
    {
        const CRYPT_BIT_BLOB* pPublicKey = &pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey;
        if (pPublicKey->cbData != publicKeySize)
        {
            return FALSE;
        }
        else if (pPublicKey->cUnusedBits != 0)
        {
            return FALSE;
        }
        else if (memcmp(pPublicKey->pbData, publicKey, publicKeySize) != 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL gfnInternalTestCertificateChain(PCCERT_CONTEXT pCertContext, const LPCSTR chain[])
{
    PCCERT_CONTEXT pCurrentCert = pCertContext, pParentCert = NULL;
    BOOL bRootCheck = FALSE;
    BOOL bMatch = FALSE;

    for (UINT i = 0; chain[i] != NULL; i++)
    {
        DWORD dwFlags = 0;
        if (!gfnInternalCheckCert(pCurrentCert, bRootCheck, chain[i], 0, NULL))
        {
            bMatch = FALSE;
            break;
        }

        if (bRootCheck)
        {
            bMatch = (chain[i + 1] == NULL);
            break;
        }

        pParentCert = pfnCertGetIssuerCertificateFromStore(pCertContext->hCertStore,
            pCurrentCert,
            pParentCert,
            &dwFlags);
        if (pParentCert)
        {
            pCurrentCert = pParentCert;
        }
        else
        {
            if (chain[i + 1] == NULL)
            {
                bMatch = TRUE;
                break;
            }
            bRootCheck = TRUE;
        }
    }

    if ((pCurrentCert != pCertContext) && !bRootCheck)
    {
        pfnCertFreeCertificateContext(pCurrentCert);
    }

    if (bMatch)
    {
        SetLastError(ERROR_SUCCESS);
    }

    return bMatch;
}

static BOOL gfnInternalIsPeNvidiaSigned(PCCERT_CONTEXT pCertContext)
{
    const LPCSTR validChains[][5] =
    {
        {
            "NVIDIA Corporation",
            "Symantec Class 3 SHA256 Code Signing CA",
            "VeriSign Class 3 Public Primary Certification Authority - G5",
            "Microsoft Code Verification Root",
            NULL
        },
        {
            "NVIDIA Corporation",
            "VeriSign Class 3 Code Signing 2010 CA",
            "VeriSign Class 3 Public Primary Certification Authority - G5",
            "Microsoft Code Verification Root",
            NULL
        },
        {
            "NVIDIA Corporation",
            "Symantec Class 3 SHA256 Code Signing CA - G2",
            "VeriSign Universal Root Certification Authority",
            "Microsoft Code Verification Root",
            NULL
        },
        {
            "NVIDIA Corporation",
            "Symantec Class 3 SHA256 Code Signing CA",
            "VeriSign Class 3 Public Primary Certification Authority - G5",
            NULL
        },
        {
            "NVIDIA Corporation",
            "VeriSign Class 3 Code Signing 2010 CA",
            "VeriSign Class 3 Public Primary Certification Authority - G5",
            NULL
        },
        {
            "NVIDIA Corporation",
            "Symantec Class 3 SHA256 Code Signing CA - G2",
            "VeriSign Universal Root Certification Authority",
            NULL
        },
        {
            "NVIDIA Corporation PE Sign v2016",
            "NVIDIA Subordinate CA 2016 v2",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {
            "NVIDIA Corporation-PE-Prod-Sha1",
            "NVIDIA Subordinate CA 2018-Prod-Sha1",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {
            "NVIDIA Corporation-PE-Prod-Sha2",
            "NVIDIA Subordinate CA 2018-Prod-Sha2",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {
            "NVIDIA Corporation-PE-Prod-Sha1",
            "NVIDIA Subordinate CA 2019-Prod-Sha1",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {
            "NVIDIA Corporation-PE-Prod-Sha2",
            "NVIDIA Subordinate CA 2019-Prod-Sha2",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {
            "NVIDIA CORPORATION",
            "NVIDIA Subordinate CA",
            "Microsoft Digital Media Authority 2005",
            NULL
        },
        {NULL}
    };

    for (UINT i = 0; validChains[i][0] != NULL; i++)
    {
        if (gfnInternalTestCertificateChain(pCertContext, validChains[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL gfnInternalIsPeGfnSigned(PCCERT_CONTEXT pCertContext)
{
    typedef struct tagCertInfo
    {
        LPCSTR subject;
        size_t publicKeySize;
        const BYTE* publicKey;
    } CertInfo;
    typedef struct tagCertChainInfo
    {
        const CertInfo certs[4];
    } CertChainInfo;
    static const BYTE c_prodL1IntermediatePublicKey[] =
    {
        0x30, 0x82, 0x02, 0x0a, 0x02, 0x82, 0x02, 0x01,
        0x00, 0xb9, 0x12, 0xcc, 0x1c, 0x99, 0x18, 0x50, 0xaa, 0x2f, 0x78, 0xfd, 0x89, 0x67, 0x8d,
        0x54, 0xa9, 0x3d, 0x7f, 0x4d, 0xc0, 0x97, 0xa6, 0x76, 0xb0, 0xd4, 0x94, 0x8a, 0xe0, 0x98,
        0xca, 0xc8, 0x9e, 0x0f, 0x21, 0xd8, 0xd4, 0x2a, 0x88, 0xa9, 0x0d, 0x53, 0xdb, 0xe6, 0x0a,
        0x60, 0x82, 0xc9, 0xed, 0x2a, 0x8d, 0xf1, 0x5f, 0x9e, 0x78, 0xeb, 0x3b, 0x8d, 0x4a, 0xfc,
        0xfb, 0x3d, 0xa6, 0xbf, 0x24, 0x3e, 0xc2, 0x6f, 0x61, 0x77, 0xa3, 0xc2, 0xfd, 0x5d, 0xa1,
        0xe6, 0xb3, 0x99, 0x55, 0x1a, 0x51, 0xcc, 0xf1, 0x44, 0xcc, 0x59, 0x8a, 0x31, 0xba, 0x20,
        0xb2, 0x63, 0x9f, 0x0c, 0xed, 0x62, 0xaa, 0xbe, 0x0e, 0xa0, 0x4e, 0xe7, 0x44, 0x6b, 0x77,
        0xa1, 0xc7, 0xca, 0x88, 0x5b, 0x20, 0x26, 0x2d, 0x39, 0xaf, 0x1f, 0x58, 0x69, 0xce, 0x13,
        0xd4, 0xe7, 0x75, 0xac, 0xb8, 0xc1, 0x44, 0xbe, 0xd9, 0x89, 0x97, 0xb7, 0x5c, 0xa7, 0xfd,
        0x15, 0xdd, 0x0e, 0x3b, 0x4c, 0xe1, 0x21, 0xab, 0xd5, 0xe3, 0x61, 0x41, 0xaf, 0xcb, 0xcb,
        0x64, 0x92, 0x1e, 0x4d, 0x49, 0x56, 0x21, 0x6a, 0x86, 0xc5, 0xd8, 0x93, 0x81, 0x13, 0xdc,
        0x1a, 0xd0, 0x30, 0x5b, 0x08, 0xdd, 0x59, 0x10, 0xcb, 0xf1, 0x88, 0xfa, 0x2b, 0xf8, 0x27,
        0x2e, 0x5f, 0xa1, 0x8b, 0x9a, 0xce, 0xda, 0x8f, 0xc6, 0x40, 0x71, 0x35, 0x5d, 0x53, 0xe8,
        0xf9, 0xa2, 0x55, 0x03, 0x09, 0x55, 0x49, 0x3d, 0xce, 0x9f, 0xe4, 0x13, 0x6c, 0xe9, 0xe6,
        0xb1, 0xff, 0xa4, 0xa0, 0x45, 0x8a, 0xd0, 0xeb, 0xae, 0xa6, 0x85, 0xcb, 0x59, 0x67, 0x1e,
        0x53, 0xe3, 0x53, 0x4e, 0x02, 0xb5, 0xd9, 0x1a, 0x5f, 0x95, 0xec, 0x40, 0x6f, 0xd7, 0x36,
        0x7b, 0x8d, 0xd9, 0x9e, 0xb8, 0xc3, 0xdf, 0xb0, 0x2b, 0x50, 0x05, 0xd5, 0x1f, 0x61, 0x3f,
        0x1d, 0x68, 0xa1, 0x05, 0x73, 0x01, 0xa9, 0x41, 0x6b, 0xde, 0xcf, 0xc6, 0xe9, 0xce, 0xf6,
        0xf8, 0x59, 0x6c, 0xd4, 0x56, 0xeb, 0xff, 0x02, 0xc9, 0x43, 0x19, 0xf6, 0xf3, 0x80, 0xc9,
        0x13, 0x51, 0x6c, 0x3c, 0xe3, 0xaf, 0x7a, 0x1c, 0x36, 0x74, 0x1b, 0xb0, 0xa5, 0xf8, 0xed,
        0xe2, 0x01, 0xf5, 0x20, 0xed, 0xa3, 0x9b, 0xed, 0xec, 0x84, 0x44, 0x61, 0xe0, 0x01, 0x5d,
        0x80, 0xe9, 0x08, 0xfc, 0xed, 0x6e, 0x7f, 0x6a, 0x95, 0xfd, 0xce, 0xef, 0x84, 0xb1, 0x8d,
        0x1b, 0x38, 0x38, 0x11, 0xd2, 0x60, 0x48, 0x27, 0x52, 0xe5, 0x5e, 0xc5, 0x75, 0xa5, 0x9c,
        0xb8, 0xff, 0xea, 0x68, 0xbc, 0x11, 0xf4, 0x1e, 0x5d, 0x15, 0x4b, 0xe6, 0x78, 0x45, 0xb9,
        0x33, 0x67, 0x8c, 0x8c, 0x0f, 0xf9, 0x0c, 0x72, 0xa4, 0xf2, 0x1a, 0xd7, 0x11, 0x4a, 0x22,
        0xca, 0xb2, 0xda, 0xc7, 0x4c, 0x76, 0xe7, 0xb5, 0xf3, 0x7c, 0xe6, 0xd9, 0x74, 0xb5, 0x08,
        0x4b, 0x0d, 0x43, 0x05, 0x17, 0x18, 0xb2, 0xd9, 0x21, 0x3b, 0x39, 0x52, 0x9d, 0x45, 0x04,
        0x6a, 0xcd, 0xfe, 0x60, 0x04, 0xc1, 0xf1, 0xbc, 0x2a, 0x3a, 0x5f, 0xab, 0xa2, 0x8c, 0xd2,
        0xcc, 0x35, 0xe6, 0xdb, 0x01, 0xb4, 0x4c, 0x95, 0x9b, 0x72, 0x7c, 0x86, 0x4c, 0x5d, 0x23,
        0x40, 0x89, 0x7d, 0x1e, 0x0f, 0xb1, 0xa0, 0xc8, 0xe1, 0x50, 0x7e, 0xd9, 0x92, 0x48, 0xbc,
        0x7f, 0xa9, 0xc5, 0xc8, 0x5c, 0x4b, 0xc6, 0x1d, 0x81, 0xc2, 0x01, 0xe3, 0xd4, 0x25, 0x6f,
        0x99, 0x95, 0x8b, 0xc4, 0x97, 0xb9, 0xf5, 0x8e, 0x0a, 0xe9, 0xc4, 0x83, 0xac, 0xc2, 0x15,
        0xca, 0x7c, 0x86, 0xd0, 0xa1, 0x96, 0xc8, 0xd1, 0x6e, 0x39, 0xb8, 0xa8, 0xdb, 0x93, 0xc4,
        0x28, 0x15, 0xb3, 0x02, 0x5f, 0x46, 0xfc, 0xae, 0xf3, 0x35, 0x01, 0xa7, 0x43, 0x66, 0x3e,
        0xc3, 0xf6, 0x45,
        0x02, 0x03, 0x01, 0x00, 0x01
    };
    static const BYTE c_prodRootPublicKey[] =
    {
        0x30, 0x82, 0x02, 0x0a, 0x02, 0x82, 0x02, 0x01,
        0x00, 0xb1, 0x4b, 0x86, 0xd8, 0x96, 0x29, 0x3a, 0x5d, 0xd7, 0xf0, 0x07, 0x07, 0xe7, 0x35,
        0xbc, 0x3f, 0xe0, 0x0b, 0x01, 0x52, 0xc2, 0x43, 0x75, 0xfe, 0x74, 0x7a, 0x25, 0xfb, 0x55,
        0x82, 0xe4, 0xa8, 0x04, 0x38, 0x57, 0xa0, 0x7d, 0x99, 0xae, 0xde, 0xa8, 0xe6, 0xcb, 0x59,
        0x06, 0x4b, 0x65, 0x8a, 0x1c, 0x19, 0x46, 0xd5, 0x85, 0x62, 0xe6, 0x3a, 0xaa, 0xc6, 0xef,
        0x46, 0xa2, 0xea, 0x53, 0x7f, 0x3c, 0xdc, 0x55, 0xf3, 0x6b, 0x40, 0xc1, 0xbb, 0x6b, 0xbb,
        0xf4, 0x96, 0xe1, 0x96, 0x63, 0xe9, 0xd7, 0x1d, 0x8b, 0xc6, 0x1a, 0xff, 0x98, 0x28, 0x9c,
        0x91, 0x90, 0x09, 0xad, 0x24, 0x37, 0x52, 0x4b, 0xa7, 0x75, 0x08, 0x8a, 0x8b, 0x35, 0x4d,
        0x10, 0x37, 0x6c, 0x61, 0x8d, 0x1d, 0xb4, 0x9c, 0xee, 0xc9, 0x76, 0xe2, 0x0b, 0x7c, 0xcd,
        0xc1, 0x60, 0x5c, 0xcf, 0x5f, 0x79, 0x07, 0x02, 0x3d, 0xa8, 0x9a, 0x6f, 0x4e, 0x18, 0xe5,
        0x51, 0xdc, 0xbd, 0xef, 0xbb, 0xac, 0x51, 0xf6, 0x9d, 0xd2, 0x49, 0x10, 0xcc, 0x30, 0x0f,
        0x43, 0xa0, 0x64, 0xf3, 0xbe, 0xfb, 0x3f, 0xbd, 0xa3, 0x54, 0x60, 0xf1, 0x91, 0x59, 0xd3,
        0x60, 0x60, 0x7b, 0xb7, 0x49, 0xa1, 0x29, 0x47, 0xd5, 0x6f, 0xe8, 0x3f, 0xbb, 0x67, 0x91,
        0x54, 0x87, 0x81, 0x75, 0x3b, 0x27, 0x46, 0xd0, 0x3b, 0xf6, 0x55, 0xf1, 0x9a, 0x48, 0x7f,
        0x06, 0xee, 0x9c, 0x9b, 0xea, 0xa4, 0xe1, 0x23, 0x87, 0x00, 0xfe, 0x61, 0xca, 0x42, 0x9c,
        0xb7, 0xdb, 0x5d, 0x85, 0xa8, 0x9d, 0x13, 0x15, 0x86, 0x30, 0xeb, 0x97, 0xc8, 0x31, 0xbe,
        0x36, 0xb2, 0x11, 0x2d, 0xa8, 0x9f, 0x91, 0x31, 0x62, 0xa8, 0xec, 0x2d, 0x4b, 0x8f, 0xcc,
        0x28, 0x2e, 0x49, 0x6e, 0xe2, 0xd3, 0x01, 0xf7, 0x00, 0x30, 0x1f, 0xfd, 0xb7, 0x56, 0xe1,
        0x80, 0x6c, 0x56, 0x9c, 0xcb, 0x3c, 0xeb, 0x4e, 0x01, 0xf7, 0x6e, 0x16, 0x70, 0xb5, 0xce,
        0x02, 0x7a, 0x30, 0xda, 0xea, 0x8e, 0xb3, 0x92, 0xd4, 0x20, 0xfd, 0xae, 0x64, 0x4a, 0xdf,
        0x32, 0xe1, 0x44, 0xb0, 0xef, 0xae, 0x6d, 0x6a, 0x44, 0xa5, 0x8f, 0xc5, 0xf5, 0xd1, 0x4f,
        0xa4, 0x5a, 0x4b, 0xa5, 0x93, 0xbc, 0x59, 0xc2, 0x6c, 0xb9, 0x84, 0x2c, 0xf2, 0x4f, 0xcf,
        0x20, 0x14, 0xa0, 0xb1, 0xfa, 0x78, 0xa2, 0xe0, 0xc4, 0xda, 0x59, 0x04, 0x4d, 0x8f, 0xbd,
        0xf4, 0x59, 0xdb, 0xf1, 0xc8, 0x34, 0x0b, 0x42, 0x69, 0x25, 0x00, 0xda, 0x65, 0xe3, 0x3d,
        0x1e, 0x2e, 0x39, 0x50, 0xb8, 0x11, 0xa0, 0x72, 0x84, 0x9b, 0x12, 0x5d, 0x18, 0x16, 0x91,
        0xfe, 0x0f, 0xf1, 0xac, 0x44, 0x11, 0xd2, 0x53, 0x41, 0x75, 0x62, 0x1e, 0xed, 0xea, 0x41,
        0xdd, 0xe0, 0x07, 0x85, 0x43, 0x44, 0x61, 0x2b, 0x57, 0x30, 0xb8, 0x17, 0xd2, 0x2a, 0x5a,
        0x0d, 0xed, 0xcd, 0xf4, 0x44, 0x05, 0x25, 0xb7, 0x05, 0x52, 0xa6, 0x1c, 0x74, 0xac, 0x5e,
        0x1a, 0x8d, 0x61, 0xc1, 0x60, 0x6f, 0x9a, 0xff, 0xdf, 0x0c, 0xbd, 0xf6, 0x67, 0x53, 0x5e,
        0x8d, 0x8b, 0x19, 0xda, 0x58, 0xe5, 0x72, 0xa8, 0xcf, 0x29, 0xf2, 0x95, 0x24, 0xdc, 0x53,
        0x46, 0x6f, 0x7f, 0x0d, 0x4e, 0x56, 0x45, 0x28, 0xfa, 0x5d, 0x66, 0x41, 0x5d, 0x49, 0xa2,
        0x02, 0x95, 0x20, 0x88, 0xd3, 0x22, 0x0e, 0x83, 0xe3, 0x75, 0xa0, 0xf9, 0xb8, 0x04, 0x25,
        0x58, 0xea, 0xf6, 0xf8, 0x49, 0x1c, 0xd3, 0xc6, 0xe1, 0x85, 0xf1, 0x02, 0x5b, 0x9f, 0x63,
        0x66, 0xd8, 0xbb, 0x74, 0x48, 0x89, 0xf6, 0xaf, 0xd2, 0x80, 0x11, 0x82, 0x84, 0x90, 0x39,
        0xa0, 0xc1, 0xf6, 0x69, 0x12, 0xb7, 0x26, 0xf9, 0xe1, 0xa9, 0x2e, 0xe3, 0xb1, 0xe1, 0x03,
        0xf8, 0xda, 0x13,
        0x02, 0x03, 0x01, 0x00, 0x01
    };
    static const CertChainInfo c_validChains[] =
    {
        {
            {
                { "GFN SDK - Code Signing Leaf (prod)", 0, NULL },
                { "GFN SDK - Code Signing Zone All CA 01", 0, NULL },
                {
                    "GFN SDK - Code Signing L1 Intermediate CA 01",
                    sizeof(c_prodL1IntermediatePublicKey),
                    c_prodL1IntermediatePublicKey
                },
                {
                    "GFN Root CA 01",
                    sizeof(c_prodRootPublicKey),
                    c_prodRootPublicKey
                }
            }
        }
    };

    BOOL bMatch = FALSE;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    CERT_ENHKEY_USAGE EnhkeyUsage;
    CERT_USAGE_MATCH CertUsage;
    CERT_CHAIN_PARA ChainPara;

    EnhkeyUsage.cUsageIdentifier = 0;
    EnhkeyUsage.rgpszUsageIdentifier = NULL;
    CertUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertUsage.Usage = EnhkeyUsage;
    ChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    ChainPara.RequestedUsage = CertUsage;

    // There should be only one chain with 4 certificates in it.
    BOOL bResult = pfnCertGetCertificateChain(
        HCCE_LOCAL_MACHINE,
        pCertContext,
        NULL,
        NULL,
        &ChainPara,
        0,
        NULL,
        &pChainContext);
    if (bResult && pChainContext->cChain == 1 && pChainContext->rgpChain[0]->cElement == _countof(c_validChains[0].certs))
    {
        for (size_t i = 0; i < _countof(c_validChains); i++)
        {
            for (size_t j = 0; j < _countof(c_validChains[i].certs); j++)
            {
                const CertInfo* pCertInfo = &c_validChains[i].certs[j];

                bMatch = gfnInternalCheckCert(
                    pChainContext->rgpChain[0]->rgpElement[j]->pCertContext,
                    FALSE,
                    pCertInfo->subject,
                    pCertInfo->publicKeySize,
                    pCertInfo->publicKey);
                if (!bMatch)
                {
                    break;
                }
            }

            if (bMatch)
            {
                break;
            }
        }
    }

    if (pChainContext != NULL)
    {
        pfnCertFreeCertificateChain(pChainContext);
    }

    if (bMatch)
    {
        SetLastError(ERROR_SUCCESS);
    }

    return bMatch;
}

#define gfnInternalGetModule(moduleName, hModule) (((NULL == hModule) && (NULL == (hModule = gfnInternalLoadSystemLibrary(moduleName, 0)))) ? FALSE : TRUE)
#define gfnInternalGetProc(hModule, procName, proc) (((NULL == proc) && (NULL == (*((FARPROC*)&proc) = GetProcAddress(hModule, procName)))) ? FALSE : TRUE)

static HMODULE hModCryptNet = NULL;
static HMODULE hModCryptBase = NULL;
static HMODULE hModWinTrust = NULL;
static HMODULE hModGDI32 = NULL;
static HMODULE hModAdvapi32 = NULL;
static HMODULE hModShell32 = NULL;
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
static BOOL gfnInteralPreloadCryptDlls()
{
    BOOL ret = gfnInternalGetModule(L"cryptnet.dll", hModCryptNet);
    if (!ret && GetLastError() != ERROR_MOD_NOT_FOUND)
    {
        ret = TRUE;
    }
    gfnInternalGetModule(L"cryptbase.dll", hModCryptBase);
    return ret;
}

static BOOL gfnInteralGetSignerInfoTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, FILETIME* pFiletime)
{
#ifndef szOID_RSA_signingTime
#define szOID_RSA_signingTime   "1.2.840.113549.1.9.5"
#endif
    DWORD dwSize = sizeof(FILETIME), n;
    for (n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
    {
        if (lstrcmpA(pSignerInfo->AuthAttrs.rgAttr[n].pszObjId, szOID_RSA_signingTime))
            continue;

        return pfnCryptDecodeObjectEx(ENCODING, szOID_RSA_signingTime,
            pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
            pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
            0, NULL, (PVOID)pFiletime, &dwSize);
    }
    return FALSE;
}

static BOOL gfnInternalGetSignerInfoTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, FILETIME* pFiletime)
{
#ifndef szOID_RSA_signingTime
#define szOID_RSA_signingTime   "1.2.840.113549.1.9.5"
#endif
    DWORD dwSize = sizeof(FILETIME), n;
    for (n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
    {
        if (lstrcmpA(pSignerInfo->AuthAttrs.rgAttr[n].pszObjId, szOID_RSA_signingTime))
            continue;

        return pfnCryptDecodeObjectEx(ENCODING, szOID_RSA_signingTime,
            pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
            pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
            0, NULL, (PVOID)pFiletime, &dwSize);
    }
    return FALSE;
}

static BOOL gfnInternalVerifyTimeStampSignerInfo(PCMSG_SIGNER_INFO pSignerInfo, HCERTSTORE hStore, FILETIME* pFiletime)
{
    BOOL bReturn = FALSE;
    BOOL bResult;
    DWORD dwSize;
    DWORD n;
    PCMSG_SIGNER_INFO pCounterSignerInfo = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    CERT_INFO CertInfo;
    FILETIME ft;

    for (n = 0; n < pSignerInfo->UnauthAttrs.cAttr && !bReturn; ++n)
    {
        if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, szOID_RSA_counterSign))
        {
            continue;
        }

        bResult = pfnCryptDecodeObjectEx(ENCODING,
            PKCS7_SIGNER_INFO,
            pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
            pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            NULL,
            (PVOID)&pCounterSignerInfo,
            &dwSize);

        if (!bResult)
        {
            goto VerifyTimeStampSignerInfoDone;
        }

        CertInfo.Issuer = pCounterSignerInfo->Issuer;
        CertInfo.SerialNumber = pCounterSignerInfo->SerialNumber;

        pCertContext = pfnCertFindCertificateInStore(hStore,
            ENCODING,
            0,
            CERT_FIND_SUBJECT_CERT,
            (PVOID)&CertInfo,
            NULL);
        if (!pCertContext)
        {
            goto VerifyTimeStampSignerInfoDone;
        }

        if (gfnInternalGetSignerInfoTimeStamp(pCounterSignerInfo, &ft))
        {
            bReturn = CompareFileTime(pFiletime, &ft) <= 0;
        }
        pfnCertFreeCertificateContext(pCertContext);

    }

VerifyTimeStampSignerInfoDone:
    SafeLocalFree(pCounterSignerInfo);
    return bReturn;
}

//
// ASN.1 Parsing for the timestamps
// http://luca.ntop.org/Teaching/Appunti/asn1.html
//
static DWORD gfnInternalPopASN1Sequence(const PBYTE pBuffer, PBYTE pTag, PBYTE* ppSeq, PDWORD pdwSeqSize)
{
    DWORD dwPosition = 0, dwSeqSize = 0;

    *pTag = pBuffer[dwPosition++];
    dwSeqSize = pBuffer[dwPosition++];
    if (dwSeqSize & 0x80)
    {
        int i = dwSeqSize & 0x7F;
        dwSeqSize = 0;
        while (i-- > 0)
            dwSeqSize = (dwSeqSize << 8) | pBuffer[dwPosition++];
    }

    *pdwSeqSize = dwSeqSize;
    *ppSeq = &pBuffer[dwPosition];
    return dwPosition + dwSeqSize;
}

static ULONGLONG gfnInternalParseASN1Timestamp(const char* pTimestamp, size_t timestampSize)
{
    const char FORMAT_PREFIX[] = "YYYYMMDDhhmmss";
    size_t i;
    ULONGLONG time = 0;

    for (i = 0; i < sizeof(FORMAT_PREFIX) - 1; i++)
    {
        if (i >= timestampSize || pTimestamp[i] < '0' || pTimestamp[i] > '9')
        {
            return 0ULL;
        }
        time = time * 10 + pTimestamp[i] - '0';
    }

    if (i >= timestampSize && pTimestamp[i] == '.')
    {
        const char FORMAT_MS[] = "sss";
        size_t j;
        for (i++, j = 0; j < sizeof(FORMAT_MS) - 1; i++, j++)
        {
            if (i >= timestampSize || pTimestamp[i] < '0' || pTimestamp[i] > '9')
            {
                return 0ULL;
            }
        }
    }

    if (i >= timestampSize || pTimestamp[i] != 'Z')
    {
        return 0ULL;
    }

    return time;
}

static ULONGLONG gfnInternalParseASN1TimestampOID(const PBYTE pBuffer, DWORD dwBufferSize, BOOL bFoundOid)
{
    const BYTE OID_TIMESAMP_TOKEN[] = { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x10, 0x01, 0x04 };

    const BYTE TAG_OID = 0x06;
    const BYTE TAG_OCTET_STRING = 0x04;
    const BYTE TAG_GENERALIZED_TIME = 0x18;

    DWORD dwPosition = 0;

    while (dwPosition < dwBufferSize)
    {
        PBYTE pCurrentSeq;
        DWORD dwSeqSize;
        BYTE Tag;
        dwPosition += gfnInternalPopASN1Sequence(&pBuffer[dwPosition], &Tag, &pCurrentSeq, &dwSeqSize);

        if (Tag & 0x20)
        {
            ULONGLONG t = gfnInternalParseASN1TimestampOID(pCurrentSeq, dwSeqSize, bFoundOid);
            if (t)
                return t;
            continue;
        }
        Tag &= 0x1F;

        if (!bFoundOid)
        {
            if (Tag == TAG_OID && dwSeqSize == sizeof(OID_TIMESAMP_TOKEN) && !memcmp(pCurrentSeq, OID_TIMESAMP_TOKEN, dwSeqSize))
            {
                dwPosition += gfnInternalPopASN1Sequence(&pBuffer[dwPosition], &Tag, &pCurrentSeq, &dwSeqSize);
                if (Tag & 0x20)
                    gfnInternalPopASN1Sequence(pCurrentSeq, &Tag, &pCurrentSeq, &dwSeqSize);
                if (Tag == TAG_OCTET_STRING)
                    return gfnInternalParseASN1TimestampOID(pCurrentSeq, dwSeqSize, TRUE);
            }
        }
        else if (Tag == TAG_GENERALIZED_TIME)
        {
            return gfnInternalParseASN1Timestamp((const char*)pCurrentSeq, (size_t)dwSeqSize);
        }
    }
    return 0ULL;
}

static BOOL gfnInternalVerifyTimeStampRFC3161(PCMSG_SIGNER_INFO pSignerInfo, FILETIME* pFiletime)
{
    BOOL bReturn = FALSE;
    BOOL bResult;
    DWORD dwSize;
    DWORD n;
    PCRYPT_CONTENT_INFO pCounterSignerInfo = NULL;
    ULONGLONG tSign, tCounterSign;

    for (n = 0; n < pSignerInfo->UnauthAttrs.cAttr; ++n)
    {
#ifndef szOID_RFC3161_counterSign
#define szOID_RFC3161_counterSign "1.3.6.1.4.1.311.3.3.1"
#endif
        if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, szOID_RFC3161_counterSign))
        {
            continue;
        }

        bResult = pfnCryptDecodeObjectEx(ENCODING,
            PKCS_CONTENT_INFO,
            pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
            pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            NULL,
            (PVOID)&pCounterSignerInfo,
            &dwSize);
        if (bResult)
        {
            tCounterSign = gfnInternalParseASN1TimestampOID(pCounterSignerInfo->Content.pbData,
                pCounterSignerInfo->Content.cbData, FALSE);
            if (tCounterSign > 0)
            {
                SYSTEMTIME st;
                FileTimeToSystemTime(pFiletime, &st);
                tSign = st.wSecond * 1ull + st.wMinute * 100ull +
                    st.wHour * 10000ull + st.wDay * 1000000ull +
                    st.wMonth * 100000000ull + st.wYear * 10000000000ull;
                bReturn = tSign <= tCounterSign;
            }
        }
        break;
    }
    SafeLocalFree(pCounterSignerInfo);
    return bReturn;
}

static LONG gfnInternalQueryRegistryValueFromKey(HKEY hKey, LPCWSTR pValueName, DWORD* pValueType, LPBYTE* ppValueData)
{
    DWORD dwValueType = 0;
    DWORD cbData = 0;
    DWORD cbExtra = 0;
    LONG lResult = ERROR_SUCCESS;

    if (!gfnInternalGetModule(L"Advapi32.dll", hModAdvapi32) ||
        !gfnInternalGetProc(hModAdvapi32, "RegQueryValueExW", pfnRegQueryValueExW))
    {
        return GetLastError();
    }

    lResult = pfnRegQueryValueExW(hKey, pValueName, NULL, &dwValueType, NULL, &cbData);
    if ((ERROR_SUCCESS != lResult) || (0 == cbData))
    {
        return lResult;
    }

    switch (dwValueType)
    {
    case REG_SZ:
    case REG_EXPAND_SZ:
        cbExtra = sizeof(WCHAR);
        break;
    case REG_MULTI_SZ:
        cbExtra = sizeof(WCHAR) + sizeof(WCHAR);
        break;
    default:
        break;
    }

    *ppValueData = (LPBYTE)LocalAlloc(LPTR, cbData + cbExtra);
    if (NULL == *ppValueData)
    {
        return GetLastError();
    }

    lResult = pfnRegQueryValueExW(hKey, pValueName, NULL, pValueType, *ppValueData, &cbData);
    if (ERROR_SUCCESS != lResult)
    {
        SafeLocalFree(*ppValueData);
        *ppValueData = NULL;
    }

    return lResult;
}

static LONG gfnInternalQueryRegistryValue(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR pValueName, DWORD* pValueType, LPBYTE* ppValueData)
{
    HKEY hSubKey = NULL;
    LONG lResult = ERROR_SUCCESS;

    if (!gfnInternalGetModule(L"Advapi32.dll", hModAdvapi32) ||
        !gfnInternalGetProc(hModAdvapi32, "RegOpenKeyExW", pfnRegOpenKeyExW) ||
        !gfnInternalGetProc(hModAdvapi32, "RegCloseKey", pfnRegCloseKey))
    {
        return GetLastError();
    }

    lResult = pfnRegOpenKeyExW(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hSubKey);
    if (ERROR_SUCCESS == lResult)
    {
        lResult = gfnInternalQueryRegistryValueFromKey(hSubKey, pValueName, pValueType, ppValueData);
        pfnRegCloseKey(hSubKey);
    }

    return lResult;
}

#ifndef WINTRUST_ACTION_GENERIC_VERIFY_V2
#define WINTRUST_ACTION_GENERIC_VERIFY_V2                       \
            { 0xaac56b,                                         \
              0xcd44,                                           \
              0x11d0,                                           \
              { 0x8c, 0xc2, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }
#endif

static LONG gfnInternalVerifySingleSignature(LPCWSTR fileName, DWORD index)
{
    WINTRUST_FILE_INFO FileData;
    WINTRUST_DATA WinTrustData;
    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    UNREFERENCED_PARAMETER(index);

    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = fileName;

    memset(&WinTrustData, 0, sizeof(WinTrustData));
    WinTrustData.cbStruct = sizeof(WinTrustData);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_IGNORE;
    WinTrustData.dwProvFlags |= WTD_CACHE_ONLY_URL_RETRIEVAL;
    WinTrustData.pFile = &FileData;

    return pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &WVTPolicyGUID, &WinTrustData);
}

static BOOL gfnInternalCryptMsgGetParam(
    HCRYPTMSG hCryptMsg,
    PCMSG_SIGNER_INFO* ppSignerInfo,
    DWORD* pdwSignerInfo)
{
    BOOL bResult = pfnCryptMsgGetParam(
        hCryptMsg,
        CMSG_SIGNER_INFO_PARAM,
        0,
        NULL,
        pdwSignerInfo);
    if (!bResult || *pdwSignerInfo == 0)
    {
        return FALSE;
    }

    *ppSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, *pdwSignerInfo);
    if (*ppSignerInfo == NULL)
    {
        return FALSE;
    }

    bResult = pfnCryptMsgGetParam(
        hCryptMsg,
        CMSG_SIGNER_INFO_PARAM,
        0,
        (PVOID)*ppSignerInfo,
        pdwSignerInfo);
    if (!bResult)
    {
        LocalFree(*ppSignerInfo);
        *ppSignerInfo = NULL;
    }

    return bResult;
}

static CONST UCHAR SG_ProtoCoded[] =
{
    0x30, 0x82,
};

static CONST UCHAR SG_SignedData[] =
{
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02,
};

#define XCH_WORD_LITEND(num) \
    (WORD)(((((WORD)num) & 0xFF00) >> 8) | ((((WORD)num) & 0x00FF) << 8))
#define _8BYTE_ALIGN(offset, base) \
    (((offset + base + 7) & 0xFFFFFFF8L) - (base & 0xFFFFFFF8L))            // Big Endian -> Little Endian

BOOL gfnInternalVerifyFileSignature(LPCWSTR fileName, SignatureType signatureType)
{
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;
    DWORD dwSignerInfo;
    BOOL bResult = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD n;
    DWORD index = 0;
    PBYTE pbCurrData = NULL;
    DWORD cbCurrData = 0x00;

    SetLastError(ERROR_SUCCESS);

    if (!gfnInternalFileExists(fileName))
    {
        dwError = ERROR_FILE_NOT_FOUND;
        goto verifyFileSignatureDone;
    }

    if (!gfnInternalGetModule(L"crypt32.dll", hModCrypt32) ||
        !gfnInternalGetProc(hModCrypt32, "CryptMsgClose", pfnCryptMsgClose) ||
        !gfnInternalGetProc(hModCrypt32, "CertCloseStore", pfnCertCloseStore) ||
        !gfnInternalGetProc(hModCrypt32, "CertFreeCertificateContext", pfnCertFreeCertificateContext) ||
        !gfnInternalGetProc(hModCrypt32, "CertFindCertificateInStore", pfnCertFindCertificateInStore) ||
        !gfnInternalGetProc(hModCrypt32, "CryptMsgGetParam", pfnCryptMsgGetParam) ||
        !gfnInternalGetProc(hModCrypt32, "CryptQueryObject", pfnCryptQueryObject) ||
        !gfnInternalGetProc(hModCrypt32, "CertGetNameStringA", pfnCertGetNameStringA) ||
        !gfnInternalGetProc(hModCrypt32, "CryptMsgOpenToDecode", pfnCryptMsgOpenToDecode) ||
        !gfnInternalGetProc(hModCrypt32, "CryptMsgUpdate", pfnCryptMsgUpdate) ||
        !gfnInternalGetProc(hModCrypt32, "CryptDecodeObjectEx", pfnCryptDecodeObjectEx) ||
        !gfnInternalGetProc(hModCrypt32, "CertGetIssuerCertificateFromStore", pfnCertGetIssuerCertificateFromStore) ||
        !gfnInternalGetProc(hModCrypt32, "CertOpenStore", pfnCertOpenStore) ||
        !gfnInternalGetProc(hModCrypt32, "CertGetCertificateChain", pfnCertGetCertificateChain) ||
        !gfnInternalGetProc(hModCrypt32, "CertFreeCertificateChain", pfnCertFreeCertificateChain) ||
        !gfnInteralPreloadCryptDlls())
    {
        dwError = ERROR_MOD_NOT_FOUND;
        goto verifyFileSignatureDone;
    }

    bResult = pfnCryptQueryObject(CERT_QUERY_OBJECT_FILE,
        fileName,
        CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
        CERT_QUERY_FORMAT_FLAG_BINARY,
        0,
        NULL,
        NULL,
        NULL,
        &hStore,
        &hMsg,
        NULL);
    if (!bResult)
    {
        dwError = GetLastError();
        goto verifyFileSignatureDone;
    }

    // Extract the primary signer
    bResult = gfnInternalCryptMsgGetParam(hMsg, &pSignerInfo, &dwSignerInfo);
    if (!bResult)
    {
        dwError = GetLastError();
        goto verifyFileSignatureDone;
    }

    bResult = gfnInternalVerifyFileSignatureInfo(pSignerInfo, hStore, fileName, index, signatureType);
    if (bResult)
    {
        dwError = ERROR_SUCCESS;
        goto verifyFileSignatureDone;
    }

    // The remaining signatures (if any) are stored in szOID_NESTED_SIGNATURE attribute in UnauthAttrs
    bResult = FALSE;
    for (n = 0; n < pSignerInfo->UnauthAttrs.cAttr; n++)
    {
        if (!lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, szOID_NESTED_SIGNATURE))
        {
            break;
        }
    }
    if (n >= pSignerInfo->UnauthAttrs.cAttr)
    {
        goto verifyFileSignatureDone;
    }
    pbCurrData = pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData;
    cbCurrData = pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData;

    // All nested signatures are stored sequentially in an 8 bytes aligned way.
    // According to the size of primary signature parse the nested signatures one by one
    while (pbCurrData > (BYTE*)pSignerInfo && pbCurrData < (BYTE*)pSignerInfo + dwSignerInfo)
    {
        HCRYPTMSG hNestedMsg = pfnCryptMsgOpenToDecode(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            0,
            0,
            NULL,
            0);
        if (!hNestedMsg) // Fatal Error
        {
            goto verifyFileSignatureDone;
        }

        // NOTE: The size in 30 82 xx doesnt contain its own size.
        // HEAD:
        // 0000: 30 82 04 df                ; SEQUENCE (4df Bytes)
        // 0004:    06 09                   ; OBJECT_ID(9 Bytes)
        // 0006:    |  2a 86 48 86 f7 0d 01 07  02
        //          |     ; 1.2.840.113549.1.7.2 PKCS 7 SignedData
        if (memcmp(pbCurrData + 0, SG_ProtoCoded, sizeof(SG_ProtoCoded)) ||
            memcmp(pbCurrData + 6, SG_SignedData, sizeof(SG_SignedData)))
        {
            break;
        }
        cbCurrData = XCH_WORD_LITEND(*(WORD*)(pbCurrData + 2)) + 4;
        PBYTE pbNextData = pbCurrData;
        pbNextData += _8BYTE_ALIGN(cbCurrData, (ULONG_PTR)pbCurrData);
        bResult = pfnCryptMsgUpdate(hNestedMsg, pbCurrData, cbCurrData, TRUE);
        pbCurrData = pbNextData;
        if (bResult)
        {
            PCMSG_SIGNER_INFO pNestedSignerInfo = NULL;
            DWORD dwNestedObjSize = 0;
            bResult = gfnInternalCryptMsgGetParam(hNestedMsg, &pNestedSignerInfo, &dwNestedObjSize);
            if (bResult)
            {
                HCERTSTORE hNestedCertStoreHandle = pfnCertOpenStore(
                    CERT_STORE_PROV_MSG,
                    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                    0,
                    0,
                    hNestedMsg);
                if (hNestedCertStoreHandle != NULL)
                {
                    bResult = gfnInternalVerifyFileSignatureInfo(
                        pNestedSignerInfo,
                        hNestedCertStoreHandle,
                        fileName,
                        ++index,
                        signatureType);
                    pfnCertCloseStore(hNestedCertStoreHandle, 0);
                }
            }
        }
        pfnCryptMsgClose(hNestedMsg);

        if (bResult)
        {
            dwError = ERROR_SUCCESS;
            goto verifyFileSignatureDone;
        }
    }

verifyFileSignatureDone:
    SafeLocalFree(pSignerInfo);
    if (hStore != NULL)
    {
        pfnCertCloseStore(hStore, 0);
    }
    if (hMsg != NULL)
    {
        pfnCryptMsgClose(hMsg);
    }
    SetLastError(dwError);
    return bResult;
}


static BOOL gfnInternalVerifyFileSignatureInfo(
    PCMSG_SIGNER_INFO pSignerInfo,
    HCERTSTORE hStore,
    LPCWSTR fileName,
    DWORD index,
    SignatureType signatureType)
{
    CERT_INFO CertInfo;
    PCCERT_CONTEXT pCertContext = NULL;
    BOOL bResult = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    FILETIME signingtime;

    CertInfo.Issuer = pSignerInfo->Issuer;
    CertInfo.SerialNumber = pSignerInfo->SerialNumber;
    pCertContext = pfnCertFindCertificateInStore(hStore,
        ENCODING,
        0,
        CERT_FIND_SUBJECT_CERT,
        (PVOID)&CertInfo,
        NULL);
    if (!pCertContext)
    {
        dwError = GetLastError();
        goto verifyFileSignatureInfoDone;
    }

    if (signatureType == SignatureTypeNvidia)
    {
        if (!gfnInternalIsPeNvidiaSigned(pCertContext))
        {
            goto verifyFileSignatureInfoDone;
        }
        dwError = GetLastError();
        if (ERROR_SUCCESS != dwError)
        {
            goto verifyFileSignatureInfoDone;
        }

        if (!gfnInteralGetSignerInfoTimeStamp(pSignerInfo, &signingtime))
        {
            memset(&signingtime, 0, sizeof(signingtime));
        }

        bResult = gfnInternalVerifyTimeStampSignerInfo(pSignerInfo, hStore, &signingtime) ||
            gfnInternalVerifyTimeStampRFC3161(pSignerInfo, &signingtime);

        if (!bResult)
        {
            dwError = GetLastError();
            if (ERROR_SUCCESS == dwError)
                dwError = (DWORD)TRUST_E_TIME_STAMP;
            goto verifyFileSignatureInfoDone;
        }
    }
    else if (signatureType == SignatureTypeGfn)
    {
        if (!gfnInternalIsPeGfnSigned(pCertContext))
        {
            dwError = GetLastError();
            goto verifyFileSignatureInfoDone;
        }
    }

    bResult = FALSE;

    if (!gfnInternalGetModule(L"wintrust.dll", hModWinTrust) ||
        !gfnInternalGetProc(hModWinTrust, "WinVerifyTrust", pfnWinVerifyTrust))
    {
        dwError = ERROR_MOD_NOT_FOUND;
        goto verifyFileSignatureInfoDone;
    }
    else
    {
        dwError = (DWORD)gfnInternalVerifySingleSignature(fileName, index);
        if (dwError == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }
    }

verifyFileSignatureInfoDone:
    if (pCertContext != NULL)
    {
        pfnCertFreeCertificateContext(pCertContext);
    }
    SetLastError(dwError);
    return bResult;
}
