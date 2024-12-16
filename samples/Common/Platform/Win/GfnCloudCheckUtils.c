// This file contains methods that facilitates CloudCheck API response validation.
// Game/application devs are free to use this implementation (*.h/*.c) files and integrate within their build system.

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

#include <GfnCloudCheckUtils.h>
#include <GfnCloudCheckAppAdapter.h>

BYTE s_RootPublicCert[] =
    "MIIF6TCCA9GgAwIBAgIUG6WcoUvnieCfcaAv8z5jEHQBT60wDQYJKoZIhvcNAQEL"
    "BQAwfDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExFDASBgNVBAcT"
    "C1NhbnRhIENsYXJhMRswGQYDVQQKExJOdmlkaWEgQ29ycG9yYXRpb24xDDAKBgNV"
    "BAsTA0dGTjEXMBUGA1UEAxMOR0ZOIFJvb3QgQ0EgMDEwHhcNMjAxMDA5MjAzNzAx"
    "WhcNNDUxMDAzMjAzNzI3WjB8MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZv"
    "cm5pYTEUMBIGA1UEBxMLU2FudGEgQ2xhcmExGzAZBgNVBAoTEk52aWRpYSBDb3Jw"
    "b3JhdGlvbjEMMAoGA1UECxMDR0ZOMRcwFQYDVQQDEw5HRk4gUm9vdCBDQSAwMTCC"
    "AiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALFLhtiWKTpd1/AHB+c1vD/g"
    "CwFSwkN1/nR6JftVguSoBDhXoH2Zrt6o5stZBktlihwZRtWFYuY6qsbvRqLqU388"
    "3FXza0DBu2u79JbhlmPp1x2Lxhr/mCickZAJrSQ3UkundQiKizVNEDdsYY0dtJzu"
    "yXbiC3zNwWBcz195BwI9qJpvThjlUdy977usUfad0kkQzDAPQ6Bk8777P72jVGDx"
    "kVnTYGB7t0mhKUfVb+g/u2eRVIeBdTsnRtA79lXxmkh/Bu6cm+qk4SOHAP5hykKc"
    "t9tdhaidExWGMOuXyDG+NrIRLaifkTFiqOwtS4/MKC5JbuLTAfcAMB/9t1bhgGxW"
    "nMs8604B924WcLXOAnow2uqOs5LUIP2uZErfMuFEsO+ubWpEpY/F9dFPpFpLpZO8"
    "WcJsuYQs8k/PIBSgsfp4ouDE2lkETY+99Fnb8cg0C0JpJQDaZeM9Hi45ULgRoHKE"
    "mxJdGBaR/g/xrEQR0lNBdWIe7epB3eAHhUNEYStXMLgX0ipaDe3N9EQFJbcFUqYc"
    "dKxeGo1hwWBvmv/fDL32Z1NejYsZ2ljlcqjPKfKVJNxTRm9/DU5WRSj6XWZBXUmi"
    "ApUgiNMiDoPjdaD5uAQlWOr2+Ekc08bhhfECW59jZti7dEiJ9q/SgBGChJA5oMH2"
    "aRK3JvnhqS7jseED+NoTAgMBAAGjYzBhMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMB"
    "Af8EBTADAQH/MB0GA1UdDgQWBBQTr3XrjyUQG9JP9EhM55kFmNVkQjAfBgNVHSME"
    "GDAWgBQTr3XrjyUQG9JP9EhM55kFmNVkQjANBgkqhkiG9w0BAQsFAAOCAgEASUJG"
    "7ap5RxndtCzlg2jHZKwXi63JhsAZr48lI54Yuc0vozPwuaBlmJ4g6B2QRHEOMzVp"
    "u/xntGAY9PaoN4N8txWBmMMAb4vGdogzdDJtPF/brk4The2NWqX1brbNzEPO8DMa"
    "3cWslpHoPOpJrviqhA0IAnLZhoBucX17kdopdPTO102F+mxmH7jcPCaILL0kyD3b"
    "NP5RLZtTmKoykEz0UwEqL4moLVHZh/0A4RTg7YlXRRAvhJvdrZXneba9L9FL/sOG"
    "O6LC4V02A+iaGMFKEYScXow+lIBsTy09eqkqYdC2CsCt/AXlcO4WIYr7xLXGA38z"
    "qi2klr02u5iamDfXpTaIApkDZbF3mBQOgi47jkeP9CO2zj2y6VGJFDMvbl8ZW18V"
    "F7HFH2jRbNNWou6Cy2EbXwVLuzYOARQVw/5p1SGmGsCMk51R0O18+0PmPm2FTUMl"
    "x4+90spza+L+0xKZSDrkirKBUoNUHwjaZ5nFvzofYhLcYJLSUIe+WYOhsQvPWvDk"
    "HcA7oYNRBlG7bcqUZHUnMU0NC3ixR+UG3lg9fvCuRH2fNPBFw8quU5aasgba1vdH"
    "wo+GYyg4Fwid4Iv0AEFYyASoNNj7BU3O6Ud4e5W8sXCqfWcYAZdNc0QWpZIRVMvs"
    "4iQUxjpe3OvvP6trvWjhkEOG5qwTLTGyBtxcQ/E=";

#define MAX_NUMBER_OF_X5C_CERTS  3
#define MAX_CERTIFICATE_CHAIN_LEN  4

/**
 * @brief Generates a random nonce.
 *
 * This function generates a random nonce by filling the provided buffer with random bytes.
 *
 * @param nonce Pointer to the buffer where the generated nonce will be stored.
 * @param nonceSize The size of the nonce buffer.
 *
 * @return true if the nonce is successfully generated, false otherwise.
 */
bool GfnCloudCheckGenerateNonce(char* nonce, unsigned int nonceSize)
{
    NTSTATUS status = S_OK;
    memset(nonce, 0, nonceSize);

    // Fill the buffer with random bytes
    status = BCryptGenRandom(NULL, (PUCHAR)nonce, nonceSize, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to generate nonce, error: %x\n", GetLastError());
        return false;
    }
    return true;
}

/**
 * @brief Decodes a Base64-encoded string and returns the decoded data.
 *
 * This function takes a Base64-encoded string and decodes it into raw binary data.
 *
 * @param src The Base64-encoded string to be decoded.
 * @param dest A pointer to the destination buffer where the decoded data will be stored.
 *             The caller is responsible for freeing this buffer.
 *
 * @return The length of the decoded data in bytes. Returns 0 if the decoding fails.
 */
static int Base64Decode(const unsigned char* src, unsigned char** dest)
{
    unsigned char decodeTable[256];
    unsigned char* output = NULL;
    unsigned char* outputPosition = 0;
    unsigned char block[4];
    unsigned char decodedCharacter;
    size_t inputLength = 0;
    size_t outputLength = 0;
    int padding = 0;
    int count = 0;
    int encodedBytes = 0;
    const unsigned char base64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Generate decode table
    memset(decodeTable, 0x80, 256);
    for (int i = 0; i < 64; i++)
    {
        decodeTable[base64Table[i]] = (unsigned char)i;
    }
    decodeTable['='] = 0;

    // Calculate length of output buffer
    inputLength = strlen(src);
    if (inputLength == 0)
    {
        GFN_CC_LOG("Empty src string\n");
        return 0;
    }
    for (unsigned int i = 0; i < inputLength; i++)
    {
        if (decodeTable[src[i]] != 0x80)
        {
            encodedBytes++;
        }
    }
    if (encodedBytes == 0)
    {
        return 0;
    }
    outputLength = (encodedBytes / 4) * 3;

    // Allocate output buffer
    output = (unsigned char*)GFN_CC_MALLOC(outputLength);
    if (!output)
    {
        GFN_CC_LOG("Failed to allocate memory for output buffer\n");
        return 0;
    }
    memset(output, 0, outputLength);

    outputPosition = output;
    for (unsigned int i = 0; i < inputLength; i++)
    {
        decodedCharacter = decodeTable[src[i]];
        if (decodedCharacter == 0x80)
        {
            continue;
        }
        if (src[i] == '=')
        {
            padding++;
        }
        block[count] = decodedCharacter;
        count++;
        if (count == 4)
        {
            *outputPosition++ = (uint8_t)((block[0] << 2) | (block[1] >> 4));
            *outputPosition++ = (uint8_t)((block[1] << 4) | (block[2] >> 2));
            *outputPosition++ = (uint8_t)((block[2] << 6) | block[3]);
            count = 0;

            if (padding != 0)
            {
                if (padding == 1)
                {
                    outputPosition--;
                }
                else if (padding == 2)
                {
                    outputPosition -= 2;
                }
                else
                {
                    GFN_CC_FREE(output);
                    return 0;
                }
                break;
            }
        }
    }
    outputLength = outputPosition - output;
    *dest = output;
    return (int)outputLength;
}

/**
 * @brief Decodes a Base64Url-encoded string and returns the decoded data.
 *
 * This function takes a Base64Url-encoded string and decodes it into raw binary data.
 *
 * @param src The Base64Url-encoded string to be decoded.
 * @param dest A pointer to the destination buffer where the decoded data will be stored.
 *             The caller is responsible for freeing this buffer.
 *
 * @return The length of the decoded data in bytes. Returns 0 if the decoding fails.
 */
static int Base64UrlDecode(const char* src, unsigned char** dest)
{
    size_t inputLength = strlen(src);
    size_t padding = 0;

    // Create a local buffer of (src length + 4) size for transformation
    char* buffer = (char* )GFN_CC_MALLOC(inputLength + 4);
    if (!buffer)
    {
        GFN_CC_LOG("Failed to allocate local buffer\n");
        return 0;
    }

    size_t bufferLength = inputLength;

    // Calculate the number of '=' characters needed to make the input size a multiple of 4
    padding = (4 - (inputLength % 4)) % 4;
    bufferLength += padding;

    // Copy the input to the result buffer and append padding characters
    strcpy(buffer, src);
    for (size_t i = 0; i < padding; i++)
    {
        buffer[inputLength + i] = '=';
    }
    buffer[bufferLength] = '\0';

    // Replace '-' with '+' and '_' with '/'
    for (size_t i = 0; i < bufferLength; i++)
    {
        if (buffer[i] == '-')
        {
            buffer[i] = '+';
        }
        else if (buffer[i] == '_')
        {
            buffer[i] = '/';
        }
    }

    // Call the Base64Decode function to decode the modified buffer
    int destSize = Base64Decode(buffer, dest);
    GFN_CC_FREE(buffer);
    return destSize;
}

/**
 * @brief Parses a JSON-formatted header string to extract information.
 * Instead of utilizing a standard open-source software, a simple custom parser is implemented
 * to allow integration into games/applications without concerns about licensing/legal issues.
 *
 * This function extracts information from a JSON-formatted header string,
 * specifically checking and parsing the "alg" and "x5c" fields.
 *
 * @param header The JSON formatted header string to be parsed.
 * @param pX5CCerts A pointer to an array of strings to store x5c certificates.
 *                   The caller is responsible for freeing each string and the array.
 * @param numOfX5CCerts A pointer to an integer to store the number of x5c certificates found.
 *                      Set to 0 if none are found or if parsing fails.
 *
 * @return true if the header is successfully parsed, false otherwise.
 */
static bool ParseHeaderJson(const unsigned char* header, const size_t headerLen, char** pX5CCerts, unsigned int* numOfX5CCerts)
{
    bool result = false;
    char* headerCopy = NULL;
    char* algValue = NULL;

    *numOfX5CCerts = 0;

    headerCopy = GFN_CC_MALLOC(sizeof(char) * (headerLen + 1));
    if (headerCopy == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for copy of header\n");
        return false;
    }
    memcpy(headerCopy, header, headerLen);
    headerCopy[headerLen] = '\0';

    const char* start = strchr(headerCopy, '{');
    const char* end = strchr(headerCopy, '}');
    if (start == NULL || end == NULL)
    {
        GFN_CC_LOG("Failed to parse start and end delimiters in the header\n");
        result = false;
        goto end;
    }
    char* algKey = strstr(start + 1, "\"alg\"");
    if (algKey == NULL)
    {
        GFN_CC_LOG("Failed to parse alg key in the header\n");
        result = false;
        goto end;
    }
    char* colon = strchr(algKey, ':');
    if (colon == NULL)
    {
        GFN_CC_LOG("Failed to parse alg key separator in the header\n");
        result = false;
        goto end;
    }
    char* algValueStart = strchr(colon + 1, '\"');
    if (algValueStart == NULL)
    {
        GFN_CC_LOG("Failed to parse alg value start in the header\n");
        result = false;
        goto end;
    }
    char* algValueEnd = strchr(algValueStart + 1, '\"');
    if (algValueEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse alg value end in the header\n");
        result = false;
        goto end;
    }
    int algLen = (int)(algValueEnd - algValueStart);
    algValue = GFN_CC_MALLOC(algLen);
    if (algValue == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for alg field in the header\n");
        result = false;
        goto end;
    }
    memset(algValue, 0, algLen);
    memcpy(algValue, algValueStart + 1, algLen - 1);
    if (strcmp(algValue, "RS512") != 0)
    {
        GFN_CC_LOG("Failed to verify alg field in the header\n");
        result = false;
        goto end;
    }

    // Parse x5c field
    char* x5cKey = strstr(start + 1, "\"x5c\"");
    if (x5cKey == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c key in the header\n");
        result = false;
        goto end;
    }
    colon = strchr(x5cKey, ':');
    if (colon == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c key separator in the header\n");
        result = false;
        goto end;
    }
    char* x5cStart = strchr(colon + 1, '[');
    if (x5cStart == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c start in the header\n");
        result = false;
        goto end;
    }
    char* x5cEnd = strchr(x5cStart + 1, ']');
    if (x5cEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c end in the header\n");
        result = false;
        goto end;
    }

    // Extract certificates
    int i = 0;
    char* x5cCert = strtok(x5cStart + 1, ",");
    while ((x5cCert != NULL) && (x5cCert < x5cEnd))
    {
        if (i >= MAX_NUMBER_OF_X5C_CERTS)
        {
            GFN_CC_LOG("Certificate count of %i exceeds expected %d\n", i, MAX_NUMBER_OF_X5C_CERTS);
            result = false;
            goto end;
        }

        size_t certLength = strlen(x5cCert);
        char* cert = GFN_CC_MALLOC(certLength + 1);
        if (cert == NULL)
        {
            GFN_CC_LOG("Failed to allocate memory for x5c cert %d\n", i);
            result = false;
            goto end;
        }
        memcpy(cert, x5cCert, certLength);
        pX5CCerts[i++] = cert;
        *numOfX5CCerts = i;

        // splitting with both , and ] to account for the last cert not having a
        // trailing ,
        x5cCert = strtok(NULL, ",]");
    }

    result = true;

end:
    if (headerCopy != NULL)
    {
        GFN_CC_FREE(headerCopy);
    }
    if (algValue != NULL)
    {
        GFN_CC_FREE(algValue);
    }
    if (!result && *numOfX5CCerts > 0)
    {
        for (unsigned int i = 0; i < *numOfX5CCerts; i++)
        {
            if (pX5CCerts[i] != NULL)
            {
                GFN_CC_FREE(pX5CCerts[i]);
                pX5CCerts[i] = NULL;
            }
        }
        *numOfX5CCerts = 0;
    }
    return result;
}

/**
 * @brief Parses a JSON-formatted payload string to verify a nonce value.
 * Instead of utilizing a standard open-source software, a simple custom parser is implemented
 * to allow integration into games/applications without concerns about licensing/legal issues.
 *
 * This function extracts and decodes the "nonce" field from a JSON-formatted payload string
 * and compares it with a provided nonce value to verify its authenticity.
 *
 * @param payload The JSON formatted payload string to be parsed.
 * @param nonce The nonce value to be compared with the decoded nonce from the payload.
 * @param nonceSize The size of nonce in bytes.
 *
 * @return true if the nonce value in the payload matches the provided nonce; false otherwise.
 */
static bool ParsePayloadJson(const unsigned char* payload, const size_t payloadLen, const char* nonce, unsigned int nonceSize)
{
    bool result = false;
    unsigned char* decodedNonce = NULL;
    char* nonceValue = NULL;
    char* payloadCopy = NULL;

    payloadCopy = GFN_CC_MALLOC(sizeof(char) * (payloadLen + 1));
    if (payloadCopy == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for copy of header\n");
        result = false;
        goto end;
    }
    memcpy(payloadCopy, payload, payloadLen);
    payloadCopy[payloadLen] = '\0';

    const char* start = strchr(payloadCopy, '{');
    const char* end = strchr(payloadCopy, '}');
    if (start == NULL || end == NULL)
    {
        GFN_CC_LOG("Failed to parse start and end delimiters in the payload\n");
        result = false;
        goto end;
    }
    char* nonceKey = strstr(start + 1, "\"nonce\"");
    if (nonceKey == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce key in the payload\n");
        result = false;
        goto end;
    }
    char* colon = strchr(nonceKey, ':');
    if (colon == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce key separator in the payload\n");
        result = false;
        goto end;
    }
    char* nonceValueStart = strchr(colon + 1, '\"');
    if (nonceValueStart == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce value start in the payload\n");
        GFN_CC_LOG("Missing nonce field\n");
        result = false;
        goto end;
    }
    char* nonceValueEnd = strchr(nonceValueStart + 1, '\"');
    if (nonceValueEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce value end in the payload\n");
        result = false;
        goto end;
    }
    int nonceLen = (int)(nonceValueEnd - nonceValueStart);
    nonceValue = GFN_CC_MALLOC(nonceLen);
    if (nonceValue == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for nonce value in the payload\n");
        result = false;
        goto end;
    }
    memset(nonceValue, 0, nonceLen);
    memcpy(nonceValue, nonceValueStart + 1, (size_t)nonceLen - 1);

    if (Base64Decode(nonceValue, &decodedNonce))
    {
        if (memcmp(decodedNonce, nonce, nonceSize) != 0)
        {
            GFN_CC_LOG("Failed to match nonce value in the payload with input nonce\n");
            result = false;
        }
        else
        {
            result = true;
        }
    }

end:
    if (payloadCopy != NULL)
    {
        GFN_CC_FREE(payloadCopy);
    }
    if (nonceValue != NULL)
    {
        GFN_CC_FREE(nonceValue);
    }
    if (decodedNonce != NULL)
    {
        GFN_CC_FREE(decodedNonce);
    }
    return result;
}

/**
 * @brief Creates an array of certificate contexts from Base64-encoded x5c certificates.
 *
 * This function takes an array of Base64-encoded x5c certificates, decodes them, and creates
 * PCCERT_CONTEXT structures for each certificate. The resulting array includes both leaf and
 * root certificates.
 *
 * @param x5cCerts An array of Base64-encoded x5c certificates.
 * @param numOfCerts The number of certificates in the x5cCerts array.
 * @param pcCertArray Pointer to the array that will store the created PCCERT_CONTEXT structures.
 *                    The caller is responsible for freeing the memory associated with this array.
 *
 * @return true if the certificate contexts are successfully created, false otherwise.
 */
bool CreateCertificateContext(char** x5cCerts, unsigned int numOfCerts, PCCERT_CONTEXT* pcCertArray)
{
    unsigned char* cert = NULL;
    int certSize = 0;
    PCCERT_CONTEXT pcCertContext = NULL;

    if (numOfCerts > MAX_CERTIFICATE_CHAIN_LEN - 1)
    {
        GFN_CC_LOG("Number of received certificates (%d) larger than limit (%d)\n", numOfCerts, MAX_CERTIFICATE_CHAIN_LEN);
        return false;
    }

    // Iterate through x5c Certificates and create the PC_CERT_CONTEXT
    for (unsigned int i = 0; i < numOfCerts; i++)
    {
        certSize = Base64Decode(x5cCerts[i], &cert);
        if (certSize == 0)
        {
            GFN_CC_LOG("Failed to decode %d x5cCertificate \n", i);
            return false;
        }
        pcCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, cert, certSize);
        if (pcCertContext == NULL)
        {
            GFN_CC_LOG("Failed to create %d certificate context, error:.%x\n", i, GetLastError());
            GFN_CC_FREE(cert);
            return false;
        }
        pcCertArray[i] = pcCertContext;
        GFN_CC_FREE(cert);
    }

    // Create the PC_CERT_CONTEXT for root certificate
    certSize = Base64Decode(s_RootPublicCert, &cert);
    if (certSize == 0)
    {
        GFN_CC_LOG("Failed to decode root certificate\n");
        return false;
    }
    pcCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, cert, certSize);
    if (pcCertContext == NULL)
    {
        GFN_CC_LOG("Failed to create root certificate context, error: %x\n", GetLastError());
        GFN_CC_FREE(cert);
        return false;
    }
    pcCertArray[numOfCerts] = pcCertContext;
    GFN_CC_FREE(cert);
    return true;
}

/**
 * @brief Generates the SHA-512 hash of the given data.
 *
 * This function uses the Windows Cryptography API to generate the SHA-512 hash
 * of the provided data.
 *
 * @param data Pointer to the data to be hashed.
 * @param dataSize Size of the data in bytes.
 * @param output Pointer to the buffer that will store the resulting hash.
 *               The caller is responsible for freeing this buffer.
 * @param outputSize Pointer to the variable that will store the size of the hash output.
 *
 * @return true if the hash is successfully generated, false otherwise.
 */
bool GenerateHash(const unsigned char* data, unsigned long dataSize, unsigned char** output, unsigned long* outputSize)
{
    NTSTATUS status = S_OK;

    // Handle to default crypto provider
    BCRYPT_ALG_HANDLE algHandle = NULL;

    // Handle to hash object
    BCRYPT_HASH_HANDLE hashObjectHandle = NULL;
    unsigned char* hashObject = NULL;
    unsigned long hashObjectLength = 0;
    unsigned long resultLength = 0;

    // Initialize OUT parameters
    *output = NULL;
    *outputSize = 0;

    // Get a handle to a cryptographic provider
    status = BCryptOpenAlgorithmProvider(&algHandle, BCRYPT_SHA512_ALGORITHM,NULL, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to open Algorithm provider, error: %x\n", GetLastError());
        goto cleanUp;
    }

    // Get size of hash object
    status = BCryptGetProperty(algHandle, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectLength, sizeof(hashObjectLength), &resultLength, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to get size of hash object, error: %x\n", GetLastError());
        goto cleanUp;
    }

    // Allocate memory for hash object
    hashObject = (unsigned char* )GFN_CC_MALLOC(hashObjectLength);
    if (hashObject == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for hash object\n");
        status = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto cleanUp;
    }

    // Create hash object
    status = BCryptCreateHash(algHandle,&hashObjectHandle, hashObject, hashObjectLength, NULL, 0, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to create hash object, error: %x\n", GetLastError());
        goto cleanUp;
    }

    // Hash the data
    status = BCryptHashData(hashObjectHandle, (PUCHAR)data, dataSize, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to generate hash of the data, error: %x\n", GetLastError());
        goto cleanUp;
    }

    // Get size of final hash buffer
    status = BCryptGetProperty(algHandle, BCRYPT_HASH_LENGTH, (PUCHAR)outputSize, sizeof(*outputSize), &resultLength, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        goto cleanUp;
    }

    // Allocate memory for hashed output
    *output = (unsigned char* )GFN_CC_MALLOC(*outputSize);
    if (*output == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for hash output\n");
        status = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto cleanUp;
    }

    // Compute the hash and receive it in the buffer
    status = BCryptFinishHash(hashObjectHandle, (PUCHAR)*output, *outputSize, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to compute the hash, error: %x\n", GetLastError());
        goto cleanUp;
    }
    status = S_OK;

cleanUp:
    if (hashObjectHandle != NULL)
    {
        BCryptDestroyHash(&hashObjectHandle);
    }
    if (hashObject != NULL)
    {
        GFN_CC_FREE(hashObject);
    }
    if (!BCRYPT_SUCCESS(status))
    {
        if (*output != NULL)
        {
            GFN_CC_FREE(*output);
        }
        *output = NULL;
        *outputSize = 0;
    }
    return status != S_OK ? false : true;
}

/**
 * @brief Validates a certificate chain composed of x5c certificates.
 *
 * This function validates a certificate chain containing x5c certificates.
 * It opens a certificate store in memory, adds the x5c certificates to the store,
 * builds the certificate chain, performs time validity checks, and verifies the chain against SSL policy.
 *
 * @param pcCertArray An array of PCCERT_CONTEXT pointers representing x5c certificates.
 *
 * @return true if the certificate chain is successfully validated, false otherwise.
 */
bool ValidateCertificateChain(PCCERT_CONTEXT* pcCertArray)
{
    bool  result = 0;
    HCERTSTORE hMemStore = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    // Open a certificate store in memory
    hMemStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, (HCRYPTPROV)NULL, 0, NULL);
    if (hMemStore == NULL)
    {
        GFN_CC_LOG("Failed to open Certificate store in memory, error: %x\n", GetLastError());
        return false;
    }

    // Add all x5c certificates to this store
    for (int i = 0; i < MAX_CERTIFICATE_CHAIN_LEN; i++)
    {
        if (!CertAddEncodedCertificateToStore(hMemStore, X509_ASN_ENCODING, pcCertArray[i]->pbCertEncoded, pcCertArray[i]->cbCertEncoded, CERT_STORE_ADD_USE_EXISTING, NULL))
        {
            GFN_CC_LOG("Failed to add %i certificate to the store, error: %x\n", i, GetLastError());
            result = false;
            goto end;
        }
    }

    // Build the certificate chain
    CERT_ENHKEY_USAGE enhkeyUsage;
    enhkeyUsage.cUsageIdentifier = 0;
    enhkeyUsage.rgpszUsageIdentifier = NULL;

    CERT_USAGE_MATCH certUsage;
    certUsage.dwType = USAGE_MATCH_TYPE_AND;
    certUsage.Usage = enhkeyUsage;

    CERT_CHAIN_PARA chainPara;
    memset(&chainPara, 0, sizeof(chainPara));
    chainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    chainPara.RequestedUsage = certUsage;

    // Get the certificate chain from the store
    if (!CertGetCertificateChain(NULL, pcCertArray[0], NULL, hMemStore, &chainPara, 0, NULL, &pChainContext))
    {
        GFN_CC_LOG("Failed to get certificate chain, error: %x\n", GetLastError());
        result = false;
        goto end;
    }
    // There should be only one chain with 4 certificates in it
    if (pChainContext->cChain == 1 && pChainContext->rgpChain[0]->cElement == MAX_CERTIFICATE_CHAIN_LEN)
    {
        for (int i = 0; i < MAX_CERTIFICATE_CHAIN_LEN; i++)
        {
            const PCCERT_CONTEXT pCurrentCertContext = pChainContext->rgpChain[0]->rgpElement[i]->pCertContext;
            if (CertVerifyTimeValidity(NULL, pCurrentCertContext->pCertInfo) != 0)
            {
                GFN_CC_LOG("Failed Cert time validation for %i certificate, error: %x\n", i, GetLastError());
                result = false;
                goto end;
            }
        }
    }
    else
    {
        GFN_CC_LOG("Invalid chain\n");
        result = false;
        goto end;
    }

    CERT_CHAIN_POLICY_PARA policyPara;
    memset(&policyPara, 0, sizeof(policyPara));
    policyPara.cbSize = sizeof(policyPara);
    policyPara.pvExtraPolicyPara = NULL;

    CERT_CHAIN_POLICY_STATUS policyStatus;
    memset(&policyStatus, 0, sizeof(policyStatus));
    policyStatus.cbSize = sizeof(policyStatus);

    if (CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChainContext, &policyPara, &policyStatus))
    {
        GFN_CC_LOG("Certificate chain verification successful!\n");
        result = true;
    }
    else
    {
        GFN_CC_LOG("Failed certificate chain verification, error: %x\n", GetLastError());
        result = false;
    }

end:
    if (pChainContext != NULL)
    {
        CertFreeCertificateChain(pChainContext);
    }
    if (hMemStore != NULL)
    {
        CertCloseStore(hMemStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
    return result;
}

/**
 * @brief Verifies the signature of hashed data using the public key from a leaf certificate.
 *
 * This function takes a decoded signature, hashed data, and the public key from a leaf certificate,
 * then verifies the signature using the public key and the specified padding algorithm.
 *
 * @param decodedSignature The decoded signature to be verified.
 * @param signatureLen The length of the decoded signature.
 * @param hashedData The hashed data that was signed.
 * @param hashedDataLen The length of the hashed data.
 * @param leafCertificate The leaf certificate containing the public key for signature verification.
 *
 * @return true if the signature is successfully verified, false otherwise.
 */
bool VerifySignature(char* decodedSignature, int signatureLen, char* hashedData, int hashedDataLen, PCCERT_CONTEXT leafCertificate)
{
    BCRYPT_KEY_HANDLE cryptKeyHandle = NULL;
    BCRYPT_PKCS1_PADDING_INFO PKCS1PaddingInfo = { 0 };
    PCCRYPT_OID_INFO pOidInfo = NULL;
    NTSTATUS status = S_OK;
    bool result = false;

    // Import public key of the Leaf certificate
    if (!(CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &leafCertificate->pCertInfo->SubjectPublicKeyInfo, 0, NULL, &cryptKeyHandle)))
    {
        GFN_CC_LOG("Failed to import public key, error %x\n", GetLastError());
        return false;
    }
    pOidInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, leafCertificate->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId, CRYPT_PUBKEY_ALG_OID_GROUP_ID);
    if (pOidInfo != NULL && lstrcmpW(pOidInfo->pwszName, L"RSA") == 0)
    {
        PKCS1PaddingInfo.pszAlgId = BCRYPT_SHA512_ALGORITHM;
    }

    status = BCryptVerifySignature(cryptKeyHandle, &PKCS1PaddingInfo, hashedData, (unsigned long)hashedDataLen, decodedSignature, (unsigned long)signatureLen, BCRYPT_PAD_PKCS1);
    if (!BCRYPT_SUCCESS(status))
    {
        GFN_CC_LOG("Failed to verify signature, error: %x\n", GetLastError());
        result = false;
    }
    else
    {
        GFN_CC_LOG("VerifySignature succeeded\n");
        result = true;
    }
    BCryptDestroyKey(cryptKeyHandle);
    return result;
}

/**
 * @brief Validates attestation data received in CloudCheck API response represented as a JWT.
 *
 * This function performs following series of steps to validate the integrity of attestation data.

 * JWT Format: base64url(header).base64url(data).base64url(RSASHA512(base64url(header).base64url(data)))
 * 1.Parse JWT to extract header, data, signature
 * 2.Parse header
 *   a.Extract Certificate chain from x5c field
 *   b.Match alg field to RS512 string
 * 3.Parse data and match nonce field with input value of nonce
 * 4.Append root certificate to the Certificate chain in #2a, and validate the Certificate chain
 * 5.Generate Hash of (base64url(header).base64url(data))
 * 6.Decrypt signature using public key of the first certificate in the list
 * 7.If decrypted signature in #6 matches with hash value in #5, indicates JWT is valid
 *
 * @param attestationData The attestation data in JWT format.
 * @param nonce The nonce value to match with the value in the payload.
 * @param nonceSize The size of nonce in bytes.
 *
 * @return true if the JWT response is valid, false otherwise.
 */
bool GfnCloudCheckVerifyAttestationData(const char* jwt, const char* nonce, unsigned int nonceSize)
{
    bool result = false;

    size_t jwtLen = 0;

    size_t headerLen = 0;
    char* header = NULL;

    size_t payloadLen = 0;
    char* payload = NULL;

    size_t signatureLen = 0;
    char* signature = NULL;

    size_t decodedHeaderLen = 0;
    unsigned char* decodedHeader = NULL;

    size_t decodedPayloadLen = 0;
    unsigned char* decodedPayload = NULL;

    size_t decodedSignatureLen = 0;
    unsigned char* decodedSignature = NULL;

    size_t dataLen = 0;
    unsigned char* data = NULL;

    char* x5cCerts[MAX_NUMBER_OF_X5C_CERTS] = { 0 };
    unsigned int numX5cCerts = 0;

    char* hashedData = NULL;
    unsigned int hashedDataLen = 0;

    char* firstDot = NULL;
    char* secondDot = NULL;

    PCCERT_CONTEXT pcCertContextArray[MAX_CERTIFICATE_CHAIN_LEN] = { 0 };

    firstDot = strchr(jwt, '.');
    if (firstDot == NULL)
    {
        GFN_CC_LOG("Invalid jwt format\n");
        return false;
    }

    secondDot = strchr(firstDot + 1, '.');
    if (secondDot == NULL)
    {
        GFN_CC_LOG("Invalid jwt format\n");
        return false;
    }

    headerLen = firstDot - jwt;
    header = GFN_CC_MALLOC(headerLen + 1);
    if (header == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for header\n");
        return false;
    }
    memcpy(header, jwt, headerLen);
    header[headerLen] = '\0';

    payloadLen = secondDot - (firstDot + 1);
    payload = GFN_CC_MALLOC(payloadLen + 1);
    if (payload == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for payload\n");
        GFN_CC_FREE(header);
        return false;
    }
    memcpy(payload, firstDot + 1, payloadLen);
    payload[payloadLen] = '\0';

    jwtLen = strlen(jwt);
    signatureLen = (jwt + jwtLen) - (secondDot + 1);

    signature = GFN_CC_MALLOC(signatureLen + 1);
    if (signature == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for signature\n");
        GFN_CC_FREE(header);
        GFN_CC_FREE(payload);
        return false;
    }
    memcpy(signature, secondDot+1, signatureLen);
    signature[signatureLen] = '\0';

    decodedHeaderLen = Base64UrlDecode(header, &decodedHeader);
    if (decodedHeaderLen == 0)
    {
        GFN_CC_LOG("Failed to Base64Url decode header\n");
        goto end;
    }

    decodedPayloadLen = Base64UrlDecode(payload, &decodedPayload);
    if (decodedPayloadLen == 0)
    {
        GFN_CC_LOG("Failed to Base64Url decode payload\n");
        goto end;
    }

    decodedSignatureLen = Base64UrlDecode(signature, &decodedSignature);
    if (decodedSignatureLen == 0)
    {
        GFN_CC_LOG("Failed to Base64Url decode signature\n");
        goto end;
    }

    if (!ParseHeaderJson(decodedHeader, decodedHeaderLen, x5cCerts, &numX5cCerts))
    {
        GFN_CC_LOG("Failed to parse header json\n");
        goto end;
    }

    if (!ParsePayloadJson(decodedPayload, decodedPayloadLen, nonce, nonceSize))
    {
        GFN_CC_LOG("Failed to parse payload json\n");
        goto end;
    }

    if (!CreateCertificateContext(x5cCerts, numX5cCerts, pcCertContextArray))
    {
        GFN_CC_LOG("Failed to create certificate context from x5c \n");
        goto end;
    }

    if (!ValidateCertificateChain(pcCertContextArray))
    {
        GFN_CC_LOG("Failed Certificate chain validation\n");
        goto end;
    }

    // To create Hash of data = (payload + header), allocate and copy data from jwt
    dataLen = secondDot - jwt;
    data = GFN_CC_MALLOC(dataLen);
    if (data == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for data object\n");
        goto end;
    }
    memcpy(data, jwt, dataLen);

    if (!GenerateHash(data, (unsigned long)dataLen, &hashedData, &hashedDataLen))
    {
        GFN_CC_LOG("Failed to generate data hash\n");
        goto end;
    }

    if (!VerifySignature(decodedSignature, (int)decodedSignatureLen, hashedData, hashedDataLen, pcCertContextArray[0]))
    {
        GFN_CC_LOG("Failed to verify signature\n");
        goto end;
    }
    else
    {
        result = true;
    }

end:
    // Free x5c certificates
    if (numX5cCerts > 0)
    {
        for (unsigned int i = 0; i < numX5cCerts; i++)
        {
            if (x5cCerts[i] != NULL)
            {
                GFN_CC_FREE(x5cCerts[i]);
            }
        }
    }
    // Free certificate context array
    for (int i = 0; i < MAX_CERTIFICATE_CHAIN_LEN; i++)
    {
        if (pcCertContextArray[i] != NULL)
        {
            CertFreeCertificateContext(pcCertContextArray[i]);
        }
    }
    if (header != NULL)
    {
        GFN_CC_FREE(header);
    }
    if (payload != NULL)
    {
        GFN_CC_FREE(payload);
    }
    if (signature != NULL)
    {
        GFN_CC_FREE(signature);
    }
    if (decodedHeader != NULL)
    {
        GFN_CC_FREE(decodedHeader);
    }
    if (decodedPayload != NULL)
    {
        GFN_CC_FREE(decodedPayload);
    }
    if (decodedSignature != NULL)
    {
        GFN_CC_FREE(decodedSignature);
    }
    if (hashedData != NULL)
    {
        GFN_CC_FREE(hashedData);
    }
    if (data != NULL)
    {
        GFN_CC_FREE(data);
    }
    return result;
}
