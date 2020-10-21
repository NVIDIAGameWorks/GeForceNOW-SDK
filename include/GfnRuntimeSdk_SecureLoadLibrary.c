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

#include "GfnRuntimeSdk_SecureLoadLibrary.h"
#include <Shlobj.h>
#include <string.h>
#include <WinTrust.h>
#include <wchar.h> // Needed for conversion to WCHAR for Win32 APIs that require it
#include <windows.h>
#include <strsafe.h>

// Internal function forward declarations
static BOOL gfnInternalFileExists(LPCWSTR szFileName);
static BOOL gfnInternalContainsAbsolutePath(LPCWSTR szFileName);
static HANDLE gfnInternalLockFileForGenericReadAccess(LPCWSTR szFilePath);
static LPWSTR gfnInternalCreateUnicodeStringFromAscii(LPCSTR szAscii);
BOOL gfnInternalVerifyFileSignature(LPCWSTR filePath, BOOL* pSignedByNvidia);

// Just emphasising that LocalFree ignores NULL args:
#define SafeLocalFree(x) LocalFree(x)
// Close HANDLE without changing last OS error:
#define SafeCloseHandle(x)                                \
    if ((NULL != (x)) && (INVALID_HANDLE_VALUE != (x))) { \
        DWORD lastError = GetLastError();                 \
        CloseHandle(x); (x) = INVALID_HANDLE_VALUE;       \
        SetLastError(lastError);                          \
    }

// ===================================================================
// Public API functions are defined in this section
// ===================================================================
HMODULE gfnSecureLoadLibraryExW(LPCWSTR filePath, DWORD dwFlags)
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
        BOOL bSignedByNvidia = FALSE;
        BOOL bSignatureVerified = gfnInternalVerifyFileSignature(filePath, &bSignedByNvidia);
        if (!bSignatureVerified || (bSignatureVerified && !bSignedByNvidia))
        {
            SafeCloseHandle(hFileLock);
            SetLastError((DWORD)CRYPT_E_NO_MATCH);
            bSignatureVerified = FALSE;
        }

        pResult = LoadLibraryExW(filePath, NULL, dwFlags);
        SafeCloseHandle(hFileLock);
    }

    return pResult;
}

HMODULE gfnSecureLoadLibraryExA(LPCSTR filePath, DWORD dwFlags)
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
    if (unicodeFilePath) {
        hResult = gfnSecureLoadLibraryExW(unicodeFilePath, dwFlags);
    }
    SafeLocalFree(unicodeFilePath);
    return hResult;
}

// ===================================================================
// Internal functions defined below. Do not use directly.
// ===================================================================

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

static BOOL gfnInternalFileExists(LPCWSTR szFileName)
{
    DWORD fileAttributes = GetFileAttributesW(szFileName);
    const DWORD nonFileAttributes = FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY;
    return ((INVALID_FILE_ATTRIBUTES == fileAttributes) || (fileAttributes & nonFileAttributes)) ? FALSE : TRUE;
}

static BOOL gfnInternalContainsAbsolutePath(LPCWSTR szFileName)
{
    if (!szFileName) {
        return FALSE;
    }

    if ((szFileName[0] == '\\') || (szFileName[0] == '/')) {
        return TRUE;
    }

    if ((isalpha(szFileName[0]) && (szFileName[1] == ':')) && ((szFileName[2] == '\\') || (szFileName[2] == '/'))) {
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

    if (!!szFileName) {

        DWORD dwLength = GetSystemDirectoryW(pResult, 0);
        pResult = (LPWSTR)LocalAlloc(LPTR, (dwLength + 1 + wcslen(szFileName)) * sizeof(WCHAR));
        if (!pResult) {
            return NULL;
        }

        dwLength = GetSystemDirectoryW(pResult, dwLength);
        if (pResult[dwLength - 1] != '\\') {
            pResult[dwLength++] = '\\';
        }

        for (i = 0; i < wcslen(szFileName); ++i) {
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

static BOOL gfnInternalTestCertificateChain(PCCERT_CONTEXT pCertContext, const LPCSTR chain[])
{
    PCCERT_CONTEXT pCurrentCert = pCertContext, pParentCert = NULL;
    BOOL bRootCheck = FALSE;
    BOOL bMatch = FALSE;
    UINT i;

    for (i = 0; chain[i] != NULL; i++)
    {
        DWORD dwFlags = 0;
        char szName[256];
        szName[0] = 0;
        pfnCertGetNameStringA(pCurrentCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            bRootCheck ? CERT_NAME_ISSUER_FLAG : 0,
            NULL,
            szName,
            sizeof(szName));

        if (strcmp(szName, chain[i]) != 0)
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
        SetLastError(ERROR_SUCCESS);

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
            return TRUE;
    }

    return FALSE;
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
        if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, szOID_RSA_counterSign)) {
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

        if (!bResult) {
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
        if (!pCertContext) {
            goto VerifyTimeStampSignerInfoDone;
        }

        if (gfnInternalGetSignerInfoTimeStamp(pCounterSignerInfo, &ft)) {
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

    for (i = 0; i < sizeof(FORMAT_PREFIX) - 1; i++) {
        if (i >= timestampSize || pTimestamp[i] < '0' || pTimestamp[i] > '9') {
            return 0ULL;
        }
        time = time * 10 + pTimestamp[i] - '0';
    }

    if (i >= timestampSize && pTimestamp[i] == '.') {
        const char FORMAT_MS[] = "sss";
        size_t j;
        for (i++, j = 0; j < sizeof(FORMAT_MS) - 1; i++, j++) {
            if (i >= timestampSize || pTimestamp[i] < '0' || pTimestamp[i] > '9') {
                return 0ULL;
            }
        }
    }

    if (i >= timestampSize || pTimestamp[i] != 'Z') {
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
        if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId, szOID_RFC3161_counterSign)) {
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

static BOOL gfnInternalIsWindowsBuildOrGreater(DWORD dwBuildNumber)
{
    static DWORD dwCurrentBuildNumber = 0;
    if (0 == dwCurrentBuildNumber)
    {
        OSVERSIONINFOEXW osvi;
        DWORDLONG dwlConditionMask = VerSetConditionMask(0, VER_BUILDNUMBER, VER_GREATER_EQUAL);
        memset(&osvi, 0, sizeof(osvi));
        osvi.dwBuildNumber = dwBuildNumber;

        if (VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, dwlConditionMask))
        {
            return TRUE;
        }
        else
        {
            DWORD dwType = 0;
            LPWSTR pszValue = NULL;
            LONG lResult = ERROR_SUCCESS;

            lResult = gfnInternalQueryRegistryValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuildNumber", &dwType, (LPBYTE*)&pszValue);
            if ((ERROR_SUCCESS == lResult) && (REG_SZ == dwType))
            {
                LPCWSTR pChar = pszValue;
                LPCWSTR pEnd = pszValue + wcslen(pszValue);
                for (; pChar < pEnd; ++pChar)
                {
                    if (isdigit(*pChar))
                    {
                        dwCurrentBuildNumber *= 10;
                        dwCurrentBuildNumber += *pChar - '0';
                    }
                    else
                    {
                        dwCurrentBuildNumber = 0;
                        break;
                    }
                }
            }

            SafeLocalFree(pszValue);
        }
    }

    return (dwCurrentBuildNumber >= dwBuildNumber) ? TRUE : FALSE;
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

static LPWSTR gfnInternalCreateSHFolderFilePath(int nFolderId, LPCWSTR szFilePath)
{
    LPWSTR pResult = NULL;
    if (!!szFilePath)
    {
        HRESULT hGetFolderPathResult;
        WCHAR szKnownFolderPath[MAX_PATH + 1];
        ZeroMemory(szKnownFolderPath, sizeof(szKnownFolderPath));

        if (NULL == hModShell32)
        {
            if (NULL != (hModShell32 = gfnInternalLoadSystemLibrary(L"Shell32.dll", 0)))
            {
                pfnSHGetFolderPath = (PfnSHGetFolderPath_W)GetProcAddress(hModShell32, "SHGetFolderPathW");
            }
        }

        if (NULL == pfnSHGetFolderPath)
        {
            return NULL;
        }

        if (S_OK == (hGetFolderPathResult = (pfnSHGetFolderPath)(NULL, nFolderId, NULL, 0, szKnownFolderPath)))
        {
            const size_t pResultChars = wcslen(szKnownFolderPath) + 1 + wcslen(szFilePath) + 1;
            if (!(pResult = (LPWSTR)LocalAlloc(LPTR, pResultChars * sizeof(WCHAR)))) {
                return NULL;
            }

            StringCchCopyW(pResult, pResultChars, szKnownFolderPath);
            StringCchCatW(pResult, pResultChars, L"\\");
            StringCchCatW(pResult, pResultChars, szFilePath);
        }
    }

    return pResult;
}

static BOOL bTrustAnyLocation = TRUE;
static OSVERSIONINFOEXW osVersionInfo = { 0 };

static BOOL gfnInternalIsTrustedLocation(LPCWSTR szFilePath)
{
    DWORD fullPathNameSize = 0;
    DWORD fullPathNameChars = 0;
    LPWSTR fullPathName = NULL;
    LPWSTR szApprovedLocation = NULL;
    BOOL bResult = FALSE;
    const int nCSIDL_FoldersToCheck[] = { CSIDL_WINDOWS, CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILESX86 };
    const int range = _countof(nCSIDL_FoldersToCheck);
    int index = 0;

    if (!osVersionInfo.dwOSVersionInfoSize)
    {
        DWORDLONG const dwlConditionMask = VerSetConditionMask(
            VerSetConditionMask(
                VerSetConditionMask(
                    0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                VER_MINORVERSION, VER_GREATER_EQUAL),
            VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        osVersionInfo.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
        osVersionInfo.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
        osVersionInfo.wServicePackMajor = 0;

        bTrustAnyLocation = VerifyVersionInfoW(&osVersionInfo, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) ? FALSE : TRUE;
    }

    if (bTrustAnyLocation)
    {
        return TRUE;
    }

    if (!szFilePath)
    {
        goto isTrustedLocationDone;
    }

    if (0 == (fullPathNameSize = GetFullPathNameW(szFilePath, 0, NULL, NULL)))
    {
        goto isTrustedLocationDone;
    }

    if (NULL == (fullPathName = (LPWSTR)LocalAlloc(LPTR, fullPathNameSize * sizeof(WCHAR))))
    {
        goto isTrustedLocationDone;
    }

    if (fullPathNameSize <= (fullPathNameChars = GetFullPathNameW(szFilePath, fullPathNameSize, fullPathName, NULL)))
    {
        goto isTrustedLocationDone;
    }

    for (index = 0; index < range; index++)
    {
#ifndef _WIN64
        if (CSIDL_PROGRAM_FILES == nCSIDL_FoldersToCheck[index])
        {
            DWORD nSize = ExpandEnvironmentStringsW(L"%ProgramW6432%", NULL, 0);
            if (!nSize || !(szApprovedLocation = (LPWSTR)LocalAlloc(LPTR, (nSize + 1) * sizeof(WCHAR))))
            {
                goto isTrustedLocationDone;
            }
            else if (nSize != ExpandEnvironmentStringsW(L"%ProgramW6432%", szApprovedLocation, nSize))
            {
                goto isTrustedLocationDone;
            }
            else if (!lstrcmpW(L"%ProgramW6432%", szApprovedLocation))
            {
                SafeLocalFree(szApprovedLocation);
                if (!(szApprovedLocation = gfnInternalCreateSHFolderFilePath(nCSIDL_FoldersToCheck[index], L"")))
                {
                    goto isTrustedLocationDone;
                }
            }
            else if (szApprovedLocation[nSize - 2] != L'\\')
            {
                StringCchCatW(szApprovedLocation, nSize + 1, L"\\");
            }
        }
        else
#endif
            if (!(szApprovedLocation = gfnInternalCreateSHFolderFilePath(nCSIDL_FoldersToCheck[index], L"")))
            {
                goto isTrustedLocationDone;
            }

        if (bResult = _wcsnicmp(fullPathName, szApprovedLocation, wcslen(szApprovedLocation)) ? FALSE : TRUE)
        {
            goto isTrustedLocationDone;
        }

        SafeLocalFree(szApprovedLocation);
        szApprovedLocation = NULL;
    }

isTrustedLocationDone:
    SafeLocalFree(szApprovedLocation);
    SafeLocalFree(fullPathName);

    return bResult;
}

static BOOL gfnInternalIsCryptSvcPausedOrRunning()
{
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS serviceStatus = { 0 };
    BOOL bIsCryptSvcPausedOrRunning = FALSE;

    if (!gfnInternalGetModule(L"Advapi32.dll", hModAdvapi32) ||
        !gfnInternalGetProc(hModAdvapi32, "OpenSCManagerW", pfnOpenSCManagerW) ||
        !gfnInternalGetProc(hModAdvapi32, "OpenServiceW", pfnOpenServiceW) ||
        !gfnInternalGetProc(hModAdvapi32, "QueryServiceStatus", pfnQueryServiceStatus) ||
        !gfnInternalGetProc(hModAdvapi32, "CloseServiceHandle", pfnCloseServiceHandle))
    {
        return FALSE;
    }

    do
    {
        hSCManager = pfnOpenSCManagerW(NULL, NULL, GENERIC_READ);
        if (NULL == hSCManager)
        {
            break;
        }
        hService = pfnOpenServiceW(hSCManager, L"CryptSvc", SERVICE_QUERY_STATUS);
        if (NULL == hService)
        {
            break;
        }
        if (FALSE == pfnQueryServiceStatus(hService, &serviceStatus))
        {
            break;
        }
        switch (serviceStatus.dwCurrentState)
        {
        case SERVICE_CONTINUE_PENDING:
        case SERVICE_PAUSE_PENDING:
        case SERVICE_PAUSED:
        case SERVICE_RUNNING:
            bIsCryptSvcPausedOrRunning = TRUE;
            break;
        }

    } while (0);

    if (NULL != hService)
    {
        pfnCloseServiceHandle(hService);
    }

    if (NULL != hSCManager)
    {
        pfnCloseServiceHandle(hSCManager);
    }

    return bIsCryptSvcPausedOrRunning;
}

static BOOL gfnInternalIsNvidiaSubordinatePublicKey(const CRYPT_BIT_BLOB* pPublicKey)
{
    const BYTE NVIDIA_Subordinate_CA_2016_v2_PublicKey[] =
    {
        0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xd3, 0x23, 0xb1,
        0xcb, 0xcb, 0xd4, 0x03, 0xc8, 0x12, 0xf0, 0x9e, 0x0a, 0x9d, 0x0b, 0xac,
        0xf8, 0x28, 0x58, 0xb8, 0x16, 0xb3, 0x89, 0x86, 0x98, 0x52, 0x86, 0xcd,
        0x5b, 0xd3, 0x83, 0xa2, 0xf7, 0x53, 0x2e, 0x1c, 0x00, 0x3d, 0x7d, 0x67,
        0xcb, 0x99, 0x86, 0x03, 0xbd, 0xb0, 0xa0, 0x8a, 0x2d, 0x92, 0x4e, 0x7f,
        0x29, 0xd1, 0xfb, 0x34, 0xe9, 0x83, 0x96, 0x16, 0x93, 0x41, 0xdc, 0x54,
        0x03, 0x62, 0x3e, 0x51, 0x0a, 0x6f, 0xae, 0xe5, 0xe8, 0xf6, 0x8b, 0x60,
        0xf6, 0x24, 0xdb, 0x41, 0x8e, 0xef, 0x20, 0xc9, 0xec, 0x8d, 0x02, 0x26,
        0xd1, 0xc9, 0x2f, 0xa0, 0xbe, 0xd9, 0xd8, 0x7d, 0x38, 0xe1, 0x59, 0x1f,
        0x37, 0xc0, 0x17, 0x84, 0x76, 0x40, 0xb3, 0xcb, 0x5a, 0x76, 0xe1, 0x5a,
        0x8c, 0x38, 0xa5, 0xa9, 0x73, 0x4c, 0xbb, 0x00, 0x92, 0x85, 0xd1, 0x7f,
        0x4e, 0x74, 0x4d, 0x19, 0x63, 0x39, 0x62, 0x62, 0xb8, 0xc3, 0x69, 0x08,
        0x67, 0xf7, 0x23, 0xfe, 0x04, 0xab, 0x9e, 0x36, 0xc9, 0x75, 0x3a, 0xc8,
        0x43, 0xd7, 0xb8, 0x0c, 0x60, 0x25, 0x79, 0x12, 0x3e, 0x05, 0x93, 0x79,
        0x45, 0x56, 0x5d, 0x41, 0xf3, 0x62, 0x43, 0x19, 0xc1, 0x5a, 0xec, 0x44,
        0x22, 0xd2, 0xd2, 0x95, 0x5d, 0xf7, 0x5c, 0x41, 0xf5, 0xdb, 0x99, 0x03,
        0x8b, 0x89, 0xa4, 0xc3, 0x93, 0xad, 0x8d, 0xa8, 0x0f, 0xa9, 0x3a, 0xf0,
        0xa3, 0xd7, 0xee, 0x81, 0xe7, 0x80, 0x4b, 0x25, 0x53, 0x7d, 0x8e, 0xfa,
        0x3d, 0x49, 0x26, 0xfc, 0xdd, 0x31, 0x4c, 0x73, 0x2d, 0xcc, 0xb7, 0x89,
        0xb7, 0xb1, 0xe8, 0x14, 0xcd, 0xd6, 0x93, 0x07, 0xf9, 0x01, 0xb7, 0xf9,
        0x35, 0xb4, 0x2f, 0x92, 0xf5, 0x9a, 0xb3, 0xd0, 0x38, 0x8d, 0x08, 0xbf,
        0x22, 0xb0, 0xa5, 0x65, 0x82, 0x90, 0x2a, 0x12, 0x55, 0xa1, 0x48, 0x11,
        0xff, 0x02, 0x03, 0x01, 0x00, 0x01
    };

    const BYTE NVIDIA_Subordinate_CA_2018_Prod_SHA1[] =
    {
        0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xa6, 0xd4, 0xc0,
        0xcf, 0x17, 0x35, 0x65, 0x8e, 0x13, 0x84, 0xda, 0xfa, 0xe4, 0xa2, 0x6e,
        0x85, 0xe7, 0xe0, 0x14, 0x98, 0x62, 0x18, 0xec, 0x27, 0x9f, 0xad, 0x55,
        0xf7, 0x9b, 0x11, 0x1a, 0x2b, 0xbe, 0x43, 0xce, 0x53, 0x67, 0x42, 0x78,
        0xb7, 0xa3, 0xd7, 0x7b, 0x74, 0xc5, 0xd6, 0x79, 0x88, 0x7f, 0x5c, 0x7d,
        0x7b, 0xda, 0x74, 0xaa, 0x97, 0xa6, 0x01, 0x93, 0x60, 0xa0, 0x59, 0xf7,
        0x05, 0x4f, 0x4b, 0xf9, 0xec, 0xfd, 0x9b, 0xff, 0xe7, 0xc4, 0xc1, 0x99,
        0x70, 0xf1, 0x29, 0x60, 0x0e, 0xe2, 0x86, 0xd2, 0x60, 0x7f, 0x52, 0xb3,
        0x17, 0x02, 0xb1, 0xe9, 0x89, 0x71, 0x86, 0x1b, 0xb9, 0xf5, 0xef, 0xd3,
        0x95, 0xa3, 0x08, 0xe5, 0x62, 0x11, 0xd7, 0x1a, 0x74, 0xa2, 0x8e, 0x52,
        0x7b, 0xc9, 0x44, 0x32, 0x99, 0xfe, 0xb9, 0xb5, 0x6f, 0x7e, 0x25, 0x37,
        0x19, 0x41, 0x58, 0x54, 0x05, 0xf5, 0x3e, 0xa5, 0x1b, 0x8f, 0x2b, 0x8f,
        0xe6, 0xeb, 0x80, 0x1e, 0x86, 0xdc, 0x89, 0x00, 0x23, 0xad, 0x29, 0xd2,
        0x4a, 0x4c, 0x25, 0xbf, 0xf7, 0x18, 0x9e, 0x24, 0x78, 0xcf, 0xbd, 0x36,
        0x11, 0x58, 0x76, 0x24, 0xdf, 0x76, 0x35, 0x01, 0xc2, 0x34, 0x7b, 0x1a,
        0x60, 0x46, 0xc6, 0x57, 0x23, 0x28, 0x4f, 0x15, 0xe7, 0x36, 0xb8, 0x1e,
        0x7a, 0xe5, 0x8a, 0xbf, 0x11, 0xc8, 0x9a, 0x90, 0x38, 0xd1, 0x0a, 0xf7,
        0xd1, 0x85, 0xfa, 0x04, 0xaf, 0xb2, 0xd3, 0x81, 0x99, 0x91, 0x5b, 0xe3,
        0xe8, 0x62, 0x15, 0xa5, 0xd0, 0xc3, 0xfc, 0x6b, 0xc6, 0x40, 0x4f, 0x8a,
        0xda, 0xf4, 0x0b, 0x57, 0x44, 0x21, 0x90, 0x48, 0xa1, 0x55, 0xbb, 0x35,
        0x75, 0xe3, 0x8b, 0x73, 0x29, 0xfa, 0x7d, 0xbf, 0x89, 0xb9, 0xf0, 0x46,
        0xcd, 0xfb, 0xab, 0xf6, 0xc1, 0xf5, 0x6b, 0x07, 0xbb, 0x76, 0x47, 0x17,
        0xe7, 0x02, 0x03, 0x01, 0x00, 0x01
    };

    const BYTE NVIDIA_Subordinate_CA_2018_Prod_SHA2[] =
    {
        0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xaf, 0x3f, 0x6a,
        0xae, 0x3b, 0x5c, 0xb6, 0x2c, 0x29, 0xfe, 0x1a, 0x3b, 0x50, 0x66, 0xfe,
        0xf7, 0x1b, 0xd0, 0x78, 0xb9, 0x30, 0x59, 0xf8, 0xb8, 0x00, 0xa8, 0x7a,
        0x67, 0x3d, 0xc5, 0x86, 0x06, 0xd9, 0x10, 0xb0, 0xe6, 0x6b, 0x4f, 0xaa,
        0xa9, 0x8d, 0xe8, 0x69, 0x60, 0xcd, 0xb1, 0x88, 0xcc, 0x19, 0x2d, 0xc5,
        0x9c, 0x51, 0xd8, 0x37, 0xb6, 0x41, 0xd7, 0x6a, 0x79, 0xf0, 0x5c, 0x6a,
        0x4e, 0xfb, 0xa2, 0xef, 0xb2, 0xd0, 0x38, 0x32, 0xd3, 0xa9, 0xe5, 0x03,
        0xeb, 0x2b, 0xec, 0xd2, 0xf0, 0x17, 0x1f, 0x6f, 0xae, 0xe1, 0xe2, 0xda,
        0x5c, 0x10, 0xb5, 0x42, 0x63, 0xbe, 0x4d, 0x0b, 0x08, 0x72, 0xc1, 0xb8,
        0xbb, 0x1a, 0x1a, 0x46, 0x21, 0xd0, 0xe7, 0xdc, 0x85, 0x36, 0xf6, 0x89,
        0x2a, 0x9d, 0xcc, 0x83, 0x59, 0x67, 0x8a, 0xfd, 0x15, 0x53, 0x4e, 0xd2,
        0x8c, 0x3a, 0x23, 0x70, 0x5d, 0x1e, 0x63, 0x39, 0x6a, 0x77, 0x54, 0x54,
        0x66, 0x6c, 0x5a, 0x24, 0x7c, 0xdd, 0x18, 0xd0, 0xc0, 0xa6, 0x7c, 0x5a,
        0xde, 0xe7, 0xa3, 0xb9, 0x4d, 0x41, 0xad, 0x6d, 0x76, 0x5f, 0x2a, 0x82,
        0x1c, 0x6f, 0x0a, 0x3c, 0x1b, 0x4b, 0x0e, 0xd4, 0x93, 0x51, 0xaf, 0x8c,
        0x04, 0x1a, 0x9e, 0x6c, 0x81, 0x34, 0x94, 0xb0, 0x20, 0xf0, 0xa1, 0x64,
        0x5a, 0x3a, 0xa7, 0x66, 0xf6, 0x66, 0x4a, 0x2e, 0xf1, 0xde, 0x85, 0x26,
        0x54, 0xa9, 0x24, 0x91, 0x3f, 0xa5, 0x1e, 0xbf, 0x44, 0x9d, 0x9c, 0x1c,
        0x28, 0xf8, 0x16, 0x2c, 0x8c, 0x31, 0xf3, 0xc3, 0x79, 0x71, 0x9e, 0x7b,
        0xe9, 0xbe, 0x96, 0x0e, 0x6a, 0x55, 0x65, 0x23, 0x54, 0xd1, 0x71, 0xea,
        0x71, 0x51, 0x42, 0xb2, 0xcc, 0x88, 0xf0, 0x91, 0x28, 0x9e, 0xc8, 0xc6,
        0xab, 0xfd, 0x24, 0x85, 0x4b, 0xa4, 0x46, 0x58, 0xae, 0xd5, 0xc3, 0xd0,
        0xd5, 0x02, 0x03, 0x01, 0x00, 0x01
    };

    const BYTE NVIDIA_Subordinate_CA_2019_Prod_SHA1[] =
    {
        0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xb4, 0x85, 0x66,
        0x85, 0xcc, 0x49, 0x35, 0x56, 0xc0, 0x91, 0xf7, 0x0b, 0xb3, 0x04, 0x34,
        0xf2, 0xab, 0x6a, 0xb9, 0xb2, 0x25, 0x43, 0x64, 0xc5, 0xa4, 0x7f, 0x27,
        0x6b, 0x7c, 0x3c, 0x91, 0x6b, 0x85, 0x0a, 0xf6, 0x0c, 0xda, 0x76, 0xd6,
        0xa5, 0xaf, 0xbb, 0xc6, 0xf0, 0xef, 0x24, 0x3c, 0x09, 0xa7, 0x04, 0xdd,
        0xfa, 0x2f, 0x3d, 0x97, 0xa2, 0x4f, 0x91, 0xdc, 0x5f, 0x58, 0x84, 0x23,
        0x7f, 0xce, 0x61, 0xa5, 0x46, 0x98, 0xcf, 0x75, 0xc7, 0x4e, 0x3e, 0x3b,
        0x7e, 0x22, 0xc8, 0xab, 0xa8, 0xbf, 0x86, 0x3f, 0x9a, 0x02, 0x70, 0xc8,
        0x30, 0x8c, 0x86, 0x21, 0x97, 0x54, 0x8b, 0xbf, 0x2d, 0x4e, 0xb6, 0xf2,
        0xb0, 0xf3, 0xf2, 0xf6, 0x0d, 0xab, 0x56, 0xac, 0x5d, 0x99, 0x63, 0x13,
        0x4a, 0x8c, 0x1e, 0xf1, 0x33, 0x9a, 0xd5, 0x41, 0xc7, 0x3f, 0xb8, 0x70,
        0x95, 0xb5, 0x1e, 0xeb, 0xb8, 0xd7, 0x2d, 0x09, 0x12, 0x34, 0x84, 0xc4,
        0xc4, 0x4c, 0x9d, 0xfe, 0xce, 0x72, 0x30, 0x82, 0x5a, 0x01, 0x93, 0xd6,
        0x3e, 0x51, 0xef, 0xf6, 0x3c, 0x01, 0x5c, 0x2d, 0xa8, 0xbe, 0x67, 0x41,
        0x0f, 0x52, 0x6d, 0xfc, 0x8c, 0x45, 0xd4, 0x3f, 0xae, 0x27, 0x89, 0x37,
        0x3c, 0x08, 0xdc, 0xd9, 0xe2, 0x7c, 0xff, 0x0b, 0xb7, 0x53, 0xb5, 0xc4,
        0xe0, 0x1d, 0x67, 0x8a, 0x4c, 0x7e, 0x5c, 0xaf, 0x41, 0xe4, 0xf0, 0x96,
        0x3e, 0xc4, 0x12, 0xde, 0xad, 0x7a, 0xb9, 0x67, 0x2c, 0x76, 0xdc, 0xfd,
        0x65, 0x51, 0x61, 0xc5, 0x6f, 0xc1, 0xb6, 0x2d, 0x4e, 0x72, 0x51, 0xa1,
        0x9f, 0xd4, 0xba, 0x3e, 0x9d, 0xea, 0xb0, 0xdc, 0xdd, 0xf4, 0x40, 0x80,
        0x82, 0x83, 0x4a, 0x2f, 0x23, 0x5d, 0xa7, 0xa0, 0xa0, 0xf5, 0x96, 0x38,
        0x48, 0xd4, 0x12, 0x4d, 0xe1, 0x99, 0xdf, 0x79, 0xcb, 0xef, 0x5d, 0xf4,
        0xaf, 0x02, 0x03, 0x01, 0x00, 0x01
    };

    const BYTE NVIDIA_Subordinate_CA_2019_Prod_SHA2[] =
    {
        0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xcf, 0x8c, 0x8d,
        0xae, 0xa1, 0x57, 0x9f, 0x14, 0x60, 0x86, 0xae, 0xd1, 0x71, 0x59, 0x1b,
        0x05, 0xfa, 0x24, 0x10, 0x45, 0xdf, 0xc9, 0x0b, 0x15, 0x94, 0x62, 0xad,
        0x20, 0xe4, 0xc4, 0xbf, 0x0f, 0x52, 0xc8, 0x9c, 0xdf, 0xae, 0xd6, 0xa9,
        0x07, 0x21, 0x98, 0xa9, 0x2f, 0x84, 0x55, 0xdd, 0x89, 0x89, 0x84, 0x22,
        0x89, 0x87, 0xd0, 0x0a, 0xcf, 0xf3, 0x2f, 0xad, 0x40, 0x80, 0x10, 0x69,
        0x24, 0xbb, 0x28, 0x47, 0x73, 0xa7, 0xa6, 0x96, 0x67, 0x39, 0x22, 0xe9,
        0x0c, 0x2c, 0x59, 0x88, 0x30, 0x86, 0xa3, 0xae, 0x8c, 0xc9, 0xd8, 0xd8,
        0xa2, 0xf9, 0xfa, 0x72, 0x3a, 0x36, 0x55, 0x73, 0xda, 0xde, 0xed, 0xae,
        0x43, 0x0f, 0xf8, 0xe7, 0xc4, 0x62, 0x48, 0xe9, 0x0a, 0x24, 0x0e, 0xc0,
        0x79, 0xb8, 0xf9, 0xc4, 0x9f, 0xb1, 0xf0, 0x52, 0x9d, 0xe4, 0x1b, 0x9f,
        0xa1, 0xa8, 0xd4, 0x67, 0xd5, 0xd6, 0x11, 0x73, 0xd8, 0xeb, 0xff, 0xa2,
        0xc0, 0xbc, 0x04, 0x6b, 0xdd, 0x46, 0x79, 0x18, 0x9c, 0xf0, 0xfd, 0xb0,
        0x3d, 0x0d, 0x28, 0x51, 0x05, 0xa5, 0x48, 0x9c, 0x81, 0x70, 0x85, 0xba,
        0x3e, 0x7f, 0x43, 0x50, 0xc1, 0x3d, 0xf1, 0x4f, 0xfa, 0xf3, 0x65, 0xcd,
        0x17, 0xbc, 0x24, 0x04, 0x51, 0x23, 0x00, 0x31, 0xd1, 0x63, 0xa3, 0x96,
        0xd5, 0x54, 0x61, 0x20, 0x47, 0x31, 0x61, 0xbf, 0xcf, 0xdd, 0xf5, 0xf8,
        0x23, 0x5d, 0x87, 0xe4, 0xe3, 0xbb, 0xe1, 0x2b, 0x30, 0x85, 0x5f, 0xd8,
        0x99, 0x16, 0xe6, 0xb0, 0x9e, 0x7a, 0xa2, 0xd0, 0x9a, 0x24, 0xe7, 0x89,
        0xee, 0xda, 0xde, 0x05, 0xd9, 0xf8, 0xc2, 0x7a, 0x0f, 0x84, 0x5f, 0xe5,
        0xb2, 0x75, 0x6c, 0x73, 0xb5, 0xf9, 0x52, 0x03, 0x48, 0x32, 0x85, 0x9e,
        0x07, 0xeb, 0x40, 0x19, 0x8e, 0x0f, 0x3d, 0x41, 0xd7, 0x54, 0xc0, 0xd2,
        0xe1, 0x02, 0x03, 0x01, 0x00, 0x01
    };

    if (pPublicKey->cbData != sizeof(NVIDIA_Subordinate_CA_2016_v2_PublicKey)) {
        return FALSE;
    }
    else if (pPublicKey->cUnusedBits != 0) {
        return FALSE;
    }

    if (!gfnInternalIsWindowsBuildOrGreater(eWindows8Build))
    {
        const BYTE Symantec_Class_3_SHA256_Code_Signing[] =
        {
            0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xd7, 0x95, 0x43,
            0xd4, 0xdc, 0xdf, 0x67, 0xae, 0x39, 0xfb, 0x52, 0xa4, 0xb6, 0x26, 0x31,
            0x04, 0x70, 0xe9, 0xb7, 0x8e, 0x5b, 0x2a, 0xba, 0x37, 0x69, 0x35, 0x95,
            0x8f, 0xbb, 0xc0, 0x30, 0xe7, 0x86, 0xd8, 0x73, 0xbb, 0xdf, 0xeb, 0xd1,
            0x76, 0x3f, 0x8a, 0x56, 0x8e, 0xeb, 0x2d, 0x4b, 0xf0, 0x57, 0x18, 0x4e,
            0xb1, 0x8d, 0xa5, 0x33, 0xd3, 0x0b, 0x75, 0x23, 0xd5, 0x6a, 0x79, 0x27,
            0xdd, 0xa3, 0xd3, 0xf7, 0x0e, 0x87, 0x65, 0xb5, 0xde, 0xad, 0x1c, 0xf1,
            0xf5, 0x35, 0xb4, 0x22, 0x51, 0xaf, 0x22, 0xa1, 0xc1, 0x5d, 0x4b, 0x90,
            0x7f, 0xc0, 0x59, 0x4e, 0xab, 0x9d, 0x79, 0xa9, 0x02, 0xd7, 0x1e, 0x49,
            0xb1, 0x3b, 0x4d, 0x87, 0xfe, 0xc2, 0x78, 0xab, 0xbf, 0xef, 0x52, 0xae,
            0x9c, 0xaf, 0x08, 0xd9, 0x39, 0xe5, 0x9a, 0x51, 0x3e, 0x69, 0x5f, 0x30,
            0x10, 0x4e, 0x71, 0x63, 0x6c, 0x58, 0xe1, 0xf0, 0x20, 0x33, 0x1b, 0x0f,
            0x74, 0xbe, 0x5b, 0xcb, 0x12, 0xe1, 0xdb, 0x8c, 0xc4, 0x80, 0x94, 0x72,
            0xbb, 0xf6, 0x45, 0x9a, 0x9e, 0xc1, 0x25, 0x0b, 0xfb, 0xa3, 0x1c, 0x9e,
            0xd8, 0xa6, 0x09, 0x70, 0x71, 0xec, 0xc0, 0x47, 0x4c, 0x8f, 0x7d, 0xc3,
            0xde, 0x19, 0xce, 0x3e, 0xee, 0x04, 0x72, 0x8e, 0x17, 0xd3, 0xff, 0xf0,
            0xb2, 0x05, 0x32, 0x19, 0x4c, 0xd3, 0xb0, 0x1c, 0x9f, 0xfe, 0xa5, 0x78,
            0x20, 0x42, 0x70, 0x41, 0xb0, 0x26, 0x8c, 0x6f, 0x00, 0xc8, 0x3a, 0x00,
            0x11, 0x41, 0x7a, 0x41, 0xb0, 0xa7, 0x8a, 0x91, 0x04, 0xa0, 0x99, 0x78,
            0xf4, 0x77, 0xb4, 0xf3, 0x60, 0x2a, 0xe6, 0x6c, 0x50, 0x04, 0xb7, 0x14,
            0x60, 0xff, 0x0d, 0x51, 0xf4, 0xa8, 0x8b, 0x84, 0xfe, 0x21, 0xda, 0x5c,
            0x5e, 0x7d, 0xf5, 0x29, 0x79, 0x4f, 0xb0, 0x44, 0x74, 0x6f, 0x6d, 0x72,
            0xff, 0x02, 0x03, 0x01, 0x00, 0x01
        };
        if (!memcmp(pPublicKey->pbData, Symantec_Class_3_SHA256_Code_Signing, pPublicKey->cbData))
            return TRUE;
    }

    return !memcmp(pPublicKey->pbData, NVIDIA_Subordinate_CA_2016_v2_PublicKey, pPublicKey->cbData) ||
        !memcmp(pPublicKey->pbData, NVIDIA_Subordinate_CA_2018_Prod_SHA1, pPublicKey->cbData) ||
        !memcmp(pPublicKey->pbData, NVIDIA_Subordinate_CA_2018_Prod_SHA2, pPublicKey->cbData) ||
        !memcmp(pPublicKey->pbData, NVIDIA_Subordinate_CA_2019_Prod_SHA1, pPublicKey->cbData) ||
        !memcmp(pPublicKey->pbData, NVIDIA_Subordinate_CA_2019_Prod_SHA2, pPublicKey->cbData);
}

BOOL gfnInternalVerifyFileSignature(LPCWSTR fileName, BOOL* pSignedByNvidia)
{
    DWORD dwEncoding, dwContentType, dwFormatType;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;
    DWORD dwSignerInfo;
    CERT_INFO CertInfo;
    PCCERT_CONTEXT pCertContext = NULL;
    BOOL bResult = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    FILETIME signingtime;

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
        !gfnInternalGetProc(hModCrypt32, "CryptDecodeObjectEx", pfnCryptDecodeObjectEx) ||
        !gfnInternalGetProc(hModCrypt32, "CertGetIssuerCertificateFromStore", pfnCertGetIssuerCertificateFromStore) ||
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
        &dwEncoding,
        &dwContentType,
        &dwFormatType,
        &hStore,
        &hMsg,
        NULL);
    if (!bResult)
    {
        dwError = GetLastError();
        goto verifyFileSignatureDone;
    }

    bResult = pfnCryptMsgGetParam(hMsg,
        CMSG_SIGNER_INFO_PARAM,
        0,
        NULL,
        &dwSignerInfo);
    if (!bResult)
    {
        dwError = GetLastError();
        goto verifyFileSignatureDone;
    }

    pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
    if (!pSignerInfo)
    {
        dwError = GetLastError();
        bResult = FALSE;
        goto verifyFileSignatureDone;
    }

    bResult = pfnCryptMsgGetParam(hMsg,
        CMSG_SIGNER_INFO_PARAM,
        0,
        (PVOID)pSignerInfo,
        &dwSignerInfo);
    if (!bResult)
    {
        dwError = GetLastError();
        goto verifyFileSignatureDone;
    }

    CertInfo.Issuer = pSignerInfo->Issuer;
    CertInfo.SerialNumber = pSignerInfo->SerialNumber;
    pCertContext = pfnCertFindCertificateInStore(hStore,
        ENCODING,
        0,
        CERT_FIND_SUBJECT_CERT,
        (PVOID)&CertInfo,
        NULL);
    if (!pCertContext) {
        dwError = GetLastError();
        bResult = FALSE;
        goto verifyFileSignatureDone;
    }

    if (NULL != pSignedByNvidia)
    {
        *pSignedByNvidia = gfnInternalIsPeNvidiaSigned(pCertContext);
        dwError = GetLastError();
        if (ERROR_SUCCESS != dwError)
        {
            goto verifyFileSignatureDone;
        }
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
        goto verifyFileSignatureDone;
    }
    if (!gfnInternalIsWindowsBuildOrGreater(eWindows8Build) &&
        gfnInternalIsTrustedLocation(fileName) &&
        !gfnInternalIsCryptSvcPausedOrRunning())
    {
        bResult = TRUE;
        dwError = ERROR_SUCCESS;
        goto verifyFileSignatureDone;
    }

    if (!gfnInternalGetModule(L"wintrust.dll", hModWinTrust) ||
        !gfnInternalGetProc(hModWinTrust, "WinVerifyTrust", pfnWinVerifyTrust))
    {
        bResult = FALSE;
        dwError = ERROR_MOD_NOT_FOUND;
        goto verifyFileSignatureDone;
    }
    else {
        DWORD i = 0;
        DWORD count = 1;
        bResult = FALSE;
        for (i = 0; i < count; i++)
        {
            LONG lResult = gfnInternalVerifySingleSignature(fileName, i);
            if (lResult == ERROR_SUCCESS)
            {
                bResult = TRUE;
                break;
            }
            dwError = (DWORD)lResult;
        }

        if (count == 1 && dwError == CERT_E_CHAINING)
        {
            DWORD dwFlags = 0;
            PCCERT_CONTEXT pIssuer = NULL;

            pIssuer = pfnCertGetIssuerCertificateFromStore(hStore, pCertContext, NULL, &dwFlags);
            if (NULL == pIssuer)
                goto verifyFileSignatureDone;

            if (gfnInternalIsNvidiaSubordinatePublicKey(&pIssuer->pCertInfo->SubjectPublicKeyInfo.PublicKey))
            {
                bResult = TRUE;
                dwError = ERROR_SUCCESS;
            }
            pfnCertFreeCertificateContext(pIssuer);
        }

        if (!bResult)
            goto verifyFileSignatureDone;
    }

verifyFileSignatureDone:
    SafeLocalFree(pSignerInfo);
    if (pCertContext != NULL) {
        pfnCertFreeCertificateContext(pCertContext);
    }
    if (hStore != NULL) {
        pfnCertCloseStore(hStore, 0);
    }
    if (hMsg != NULL) {
        pfnCryptMsgClose(hMsg);
    }
    SetLastError(dwError);
    return bResult;
}