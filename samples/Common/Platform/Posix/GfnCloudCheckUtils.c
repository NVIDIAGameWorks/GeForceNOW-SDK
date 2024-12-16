// This file contains methods that facilitates CloudCheck API response validation.
// Game/application devs are free to use this implementation (*.h/*.c) files and integrate within their build system.

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/safestack.h>

#include <GfnCloudCheckUtils.h>
#include <GfnCloudCheckAppAdapter.h>

char s_RootPublicCert[] =
    "-----BEGIN CERTIFICATE-----\n"
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
    "4iQUxjpe3OvvP6trvWjhkEOG5qwTLTGyBtxcQ/E="
    "\n-----END CERTIFICATE-----";


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
bool GfnCloudCheckGenerateNonce(char* buffer, unsigned int bufferSize)
{
    if (RAND_bytes((unsigned char*)buffer, bufferSize) != 1)
    {
        GFN_CC_LOG("Failed to generate nonce, error: %zu\n", ERR_get_error());
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
static int Base64Decode(const char* src, unsigned char** dest)
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
    for (size_t i = 0; i < inputLength; i++)
    {
        if (decodeTable[(uint8_t)src[i]] != 0x80)
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
    for (size_t i = 0; i < inputLength; i++)
    {
        decodedCharacter = decodeTable[(uint8_t)src[i]];
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
    return outputLength;
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
    size_t bufferLength = 0;
    size_t padding = 0;
    int destSize = 0;

    // Create a local buffer of (src length + 4) size for transformation
    char* buffer = GFN_CC_MALLOC(inputLength + 4);
    if (!buffer)
    {
        GFN_CC_LOG("Failed to allocate local buffer\n");
        return 0;
    }

    bufferLength = inputLength;

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
    destSize = Base64Decode(buffer, dest);
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
 * @param header The JSON formatted header bytes to be parsed.
 * @param headerLen The length of the JSON formatted header bytes to be parsed.
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
    char *headerCopy = NULL;
    char *algValue = NULL;
    size_t x5cCertLen = 0;
    size_t pemCertLen = 0;
    char* pemCert = NULL;
    char* pemCertPtr = NULL;
    const char* start = NULL;
    const char* end = NULL;
    char* algKey = NULL;
    char* colon = NULL;
    char* algValueStart = NULL;
    char* algValueEnd = NULL;
    char* x5cKey = NULL;
    int algLen = 0;
    char* x5cStart = NULL;
    char* x5cEnd = NULL;
    int i = 0;
    char* x5cCert = NULL;

    const char PemHeader[] = "-----BEGIN CERTIFICATE-----\n";
    const char PemTrailer[] = "\n-----END CERTIFICATE-----";
    size_t pemHeaderLen = 0;
    size_t pemTrailerLen = 0;

    *numOfX5CCerts = 0;

    headerCopy = GFN_CC_MALLOC(sizeof(char) * (headerLen + 1));
    if (headerCopy == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for copy of header\n");
        return false;
    }
    memcpy(headerCopy, header, headerLen);
    headerCopy[headerLen] = '\0';

    start = strchr(headerCopy, '{');
    end = strchr(headerCopy, '}');
    if (start == NULL || end == NULL)
    {
        GFN_CC_LOG("Failed to parse start and end delimiters in the header\n");
        result = false;
        goto end;
    }
    algKey = strstr(start + 1, "\"alg\"");
    if (algKey == NULL)
    {
        GFN_CC_LOG("Failed to parse alg key in the header\n");
        result = false;
        goto end;
    }
    colon = strchr(algKey, ':');
    if (colon == NULL)
    {
        GFN_CC_LOG("Failed to parse alg key separator in the header\n");
        result = false;
        goto end;
    }
    algValueStart = strchr(colon + 1, '\"');
    if (algValueStart == NULL)
    {
        GFN_CC_LOG("Failed to parse alg value start in the header\n");
        result = false;
        goto end;
    }
    algValueEnd = strchr(algValueStart + 1, '\"');
    if (algValueEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse alg value end in the header\n");
        result = false;
        goto end;
    }
    algLen = algValueEnd - algValueStart;
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
    x5cKey = strstr(start + 1, "\"x5c\"");
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
    x5cStart = strchr(colon + 1, '[');
    if (x5cStart == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c start in the header\n");
        result = false;
        goto end;
    }
    x5cEnd = strchr(x5cStart + 1, ']');
    if (x5cEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse x5c end in the header\n");
        result = false;
        goto end;
    }

    // Extract certificates
    i = 0;
    x5cCert = strtok(x5cStart + 1, ",");

    pemHeaderLen = strlen(PemHeader);
    pemTrailerLen = strlen(PemTrailer);

    while ((x5cCert != NULL) && (x5cCert < x5cEnd))
    {
        if (i >= MAX_NUMBER_OF_X5C_CERTS)
        {
            GFN_CC_LOG("Certificate count of %i exceeds expected %d\n", i, MAX_NUMBER_OF_X5C_CERTS);
            result = false;
            goto end;
        }

        x5cCertLen = strlen(x5cCert) - 2; // don't count leading and trailing '"'

        pemCertLen = x5cCertLen + pemHeaderLen + pemTrailerLen + 1;

        pemCert = GFN_CC_MALLOC(pemCertLen);
        if (pemCert == NULL)
        {
            GFN_CC_LOG("Failed to allocate memory for x5c cert %d\n", i);
            result = false;
            goto end;
        }
        memset(pemCert, '\0', pemCertLen);

        pemCertPtr = pemCert;

        memcpy(pemCertPtr, PemHeader, pemHeaderLen);
        pemCertPtr += pemHeaderLen;
        memcpy(pemCertPtr, x5cCert+1, x5cCertLen);
        pemCertPtr += x5cCertLen;
        memcpy(pemCertPtr, PemTrailer, pemTrailerLen);
        pemCertPtr += pemTrailerLen;


        pX5CCerts[i++] = pemCert;
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
        for (size_t i = 0; i < *numOfX5CCerts; i++)
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
 * @param payload The JSON formatted payload bytes to be parsed.
 * @param payloadLen The length of the JSON formatted payload bytes to be parsed.
 * @param nonce The nonce value to be compared with the decoded nonce from the payload.
 * @param nonceSize The size of nonce in bytes.
 *
 * @return true if the nonce value in the payload matches the provided nonce; false otherwise.
 */
static bool ParsePayloadJson(const unsigned char* payload, const size_t payloadLen, const char* nonce, unsigned int nonceSize)
{
    bool result = false;
    unsigned char* decodedNonce = NULL;

    char *payloadCopy = NULL;
    const char* start = NULL;
    const char* end = NULL;
    char* nonceKey = NULL;
    char* colon = NULL;
    char* nonceValueStart = NULL;
    char* nonceValueEnd = NULL;
    int nonceLen = 0;
    char* nonceValue = NULL;

    payloadCopy = GFN_CC_MALLOC(sizeof(char) * (payloadLen + 1));
    if (payloadCopy == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for copy of header\n");
        result = false;
        goto end;
    }
    memcpy(payloadCopy, payload, payloadLen);
    payloadCopy[payloadLen] = '\0';

    start = strchr(payloadCopy, '{');
    end = strchr(payloadCopy, '}');
    if (start == NULL || end == NULL)
    {
        GFN_CC_LOG("Failed to parse start and end delimiters in the payload\n");
        result = false;
        goto end;
    }
    nonceKey = strstr(start + 1, "\"nonce\"");
    if (nonceKey == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce key in the payload\n");
        result = false;
        goto end;
    }
    colon = strchr(nonceKey, ':');
    if (colon == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce key separator in the payload\n");
        result = false;
        goto end;
    }
    nonceValueStart = strchr(colon + 1, '\"');
    if (nonceValueStart == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce value start in the payload\n");
        GFN_CC_LOG("Missing nonce field\n");
        result = false;
        goto end;
    }
    nonceValueEnd = strchr(nonceValueStart + 1, '\"');
    if (nonceValueEnd == NULL)
    {
        GFN_CC_LOG("Failed to parse nonce value end in the payload\n");
        result = false;
        goto end;
    }
    nonceLen = nonceValueEnd - nonceValueStart;
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

/*
 * @brief Create an OpenSSL X.509 object from a PEM certificate string.
 *
 * @param certStr The PEM certificate string to be converted into the X.509 object.
 * @param certStrLen The length of the PEM certificate string
 * @param outputCert Storage location for the X.509 object
 *
 * @return true if X.509 object is constructed successfully, false otherwise.
 */
static bool CreateX509Cert(char *certStr, const size_t certStrLen, X509 **outputCert)
{
    bool result = false;
    BIO *certBio = NULL;
    X509 *cert = NULL;

    certBio = BIO_new(BIO_s_mem());
    if (certBio == NULL)
    {
        GFN_CC_LOG("Unable to create a bio object\n");
        goto end;
    }

    if (BIO_write(certBio, certStr, certStrLen) <= 0)
    {
        GFN_CC_LOG("Unable to write certificate to bio object\n");
        goto end;
    }

    cert = PEM_read_bio_X509(certBio, NULL, 0, NULL);
    if (cert == NULL)
    {
        GFN_CC_LOG("Unable to parse certificate\n");
        goto end;
    }

    *outputCert = cert;
    result = true;

end:
    BIO_free(certBio);

    return result;
}

/*
 * @brief Adds a certificate to a certificate chain
 *
 * Creates a X509 certificate from a certificate string and adds it to the certificate chain
 *
 * @param cert The PEM certificate string to be added to the certificate chain
 * @param certChain The chain to which the certificate is added.
 *
 * @return true if the certificate is added successfully, false otherwise.
 */
static bool AddCertificateToChain(char *cert, STACK_OF(X509) *certChain)
{
    bool result = false;
    X509 *certX509 = NULL;

    if (!CreateX509Cert(cert, strlen(cert), &certX509))
    {
        GFN_CC_LOG("Failed to create X509 certificate\n");
        goto end;
    }

    if (sk_X509_push(certChain, certX509) == 0)
    {
        GFN_CC_LOG("Failed to push X509 certificate to certificate stack\n");
        goto end;
    }

    result = true;

end:
    if (result)
    {
        return result;
    }

    X509_free(certX509);

    return result;
}

/*
 * @brief Create a certificate chain containing the passed in certificates
 *
 * Creates a STACK_OF(X509) from the passed in certificate strings
 *
 * @param x5cCerts The certificate strings received from the cloud check response JWT
 * @param numX5cCerts The number of certificate strings received from the cloud check response JWT
 * @param rootCert The root certificate to be appended to this certificate chain
 * @param certChain The output STACK_OF(X509) certificate chain
 *
 * @return true if the certificate chain is created successfully, false otherwise.
 */
static bool CreateX509CertificateChain(char *x5cCerts[], size_t numX5cCerts, char *rootCert, STACK_OF(X509) **certChain)
{
    bool result = false;

    STACK_OF(X509) *chain = NULL;

    chain = sk_X509_new_null();
    if (chain == NULL)
    {
        GFN_CC_LOG("Failed to create certificate stack for chain\n");
        goto end;
    }

    for (size_t i = 0; i < numX5cCerts; ++i)
    {
        if (!AddCertificateToChain(x5cCerts[i], chain))
        {
            GFN_CC_LOG("Failed to add (%zu) received certificate\n", i);
            goto end;
        }
    }

    if (!AddCertificateToChain(rootCert, chain))
    {
        GFN_CC_LOG("Failed to add root certificate\n");
        goto end;
    }


    *certChain = chain;
    result = true;

end:
    if (result)
    {
        return result;
    }

    sk_X509_pop_free(chain, X509_free);

    return result;
}

/*
 * @brief Verifies the received certificates against the pinned root certificate.
 *
 * @param certChain X.509 stack of certificates to verify. Expected to contain target (leaf) + intermediate + root
 *
 * @return true if the certificate chain is validated successfully, false otherwise.
 */
static bool VerifyX509CertificateChain(STACK_OF(X509) *certChain)
{
    bool result = false;
    size_t numCerts = 0;
    X509 *leafCertX509 = NULL;
    X509 *rootCertX509 = NULL;

    STACK_OF(X509) *untrustedCertsX509 = NULL;
    X509_STORE *certStore = NULL;
    X509_STORE_CTX *certStoreCtx = NULL;
    X509_VERIFY_PARAM *certChainVerifyParams = NULL;

    numCerts = sk_X509_num(certChain);
    if (numCerts <= 0)
    {
        GFN_CC_LOG("Certificate chain empty\n");
        goto end;
    }

    if (numCerts > MAX_CERTIFICATE_CHAIN_LEN)
    {
        GFN_CC_LOG("Certificate chain too long\n");
        goto end;
    }

    // get leaf (target)
    leafCertX509 = sk_X509_value(certChain, 0);
    if (leafCertX509 == NULL)
    {
        GFN_CC_LOG("Failed to get the leaf certificate\n");
        goto end;
    }

    // get root certificate
    rootCertX509 = sk_X509_value(certChain, numCerts-1);
    if (rootCertX509 == NULL)
    {
        GFN_CC_LOG("Failed to get the root certificate\n");
        goto end;
    }

    // Create untrusted chain (list of certificates that can be used to build the certificate chain)
    untrustedCertsX509 = sk_X509_dup(certChain);
    if (untrustedCertsX509 == NULL)
    {
        GFN_CC_LOG("Failed to create a copy of the certificate chain\n");
        goto end;
    }

    // remove the target and the root certificate
    if (sk_X509_shift(untrustedCertsX509) == NULL)
    {
        GFN_CC_LOG("Failed to remove leaf certificate from certificate chain copy\n");
        goto end;
    }
    if (sk_X509_pop(untrustedCertsX509) == NULL)
    {
        GFN_CC_LOG("Failed to remove root certificate from certificate chain copy\n");
        goto end;
    }

    // create store
    certStore = X509_STORE_new();
    if (certStore == NULL)
    {
        GFN_CC_LOG("Failed to create a certificate store\n");
        goto end;
    }

    if (X509_STORE_add_cert(certStore, rootCertX509) == 0)
    {
        GFN_CC_LOG("Failed to add root certificate to certificate store\n");
        goto end;
    }


    // Create context for the chain verification
    certStoreCtx = X509_STORE_CTX_new();
    if (certStoreCtx == NULL)
    {
        GFN_CC_LOG("Failed to create context for X509 store\n");
        goto end;
    }


    if (X509_STORE_CTX_init(certStoreCtx, certStore, leafCertX509, untrustedCertsX509) == 0)
    {
        GFN_CC_LOG("Failed to initialize context for X509 store\n");
        goto end;
    }

    // Create parameters for the chain verification
    certChainVerifyParams = X509_VERIFY_PARAM_new();
    if (X509_VERIFY_PARAM_set_flags(certChainVerifyParams, X509_V_FLAG_X509_STRICT) == 0)
    {
        GFN_CC_LOG("Failed to set strict verification flag for certificate chain\n");
        goto end;
    }

    X509_VERIFY_PARAM_set_depth(certChainVerifyParams, MAX_CERTIFICATE_CHAIN_LEN-2); // only count intermediate certs

    // Enable additional checks based on keyUsage, extendedKeyUsage, and basicConstraints
    if (X509_VERIFY_PARAM_set_purpose(certChainVerifyParams, X509_PURPOSE_SSL_SERVER) == 0)
    {
        GFN_CC_LOG("Failed to set purpose for certificate chain\n");
        goto end;
    }

    X509_STORE_CTX_set0_param(certStoreCtx, certChainVerifyParams);
    certChainVerifyParams = NULL;

    if (X509_STORE_CTX_verify(certStoreCtx) <= 0)
    {
        int error = X509_STORE_CTX_get_error(certStoreCtx);
        GFN_CC_LOG("Certificate chain validation failed: %s at depth: %d\n",
                X509_verify_cert_error_string(error), X509_STORE_CTX_get_error_depth(certStoreCtx));
        goto end;
    }

    result = true;

end:
    sk_X509_free(untrustedCertsX509);
    X509_STORE_free(certStore);
    X509_STORE_CTX_free(certStoreCtx);
    X509_VERIFY_PARAM_free(certChainVerifyParams);

    return result;
}

/**
 * @brief Verifies the signature of data using the public key from a leaf certificate.
 *
 * @param data The data that was signed.
 * @param dataLen The length of the signed data.
 * @param signature The signature to be verified.
 * @param signatureLen The length of the signature.
 * @param certChain The certificate stack containing the leaf certificate.
 *
 * @return true if the signature is successfully verified, false otherwise.
 */
static bool VerifySignature(const unsigned char *data, size_t dataLen, const unsigned char *signature, size_t signatureLen, STACK_OF(X509) *certChain)
{
    bool result = false;

    EVP_MD_CTX *digestVerificationCtx = NULL;
    EVP_PKEY *leafPubKey = NULL;
    int verifyStatus = 0;

    X509 *leafCertifcateX509 = sk_X509_value(certChain, 0);
    if (leafCertifcateX509 == NULL)
    {
        GFN_CC_LOG("Failed to get the leaf certificate\n");
        goto end;
    }

    leafPubKey = X509_get0_pubkey(leafCertifcateX509);
    if (leafPubKey == NULL)
    {
        GFN_CC_LOG("Failed to get the leaf public key\n");
        goto end;
    }

    digestVerificationCtx = EVP_MD_CTX_new();
    if (digestVerificationCtx == NULL)
    {
        GFN_CC_LOG("Failed to create message digest context\n");
        goto end;
    }

    if (EVP_DigestVerifyInit(digestVerificationCtx, NULL, EVP_sha512(), NULL, leafPubKey) == 0)
    {
        GFN_CC_LOG("Failed to initialize message digest verification context\n");
        goto end;
    }

    verifyStatus = EVP_DigestVerify(digestVerificationCtx, (const unsigned char*)signature, signatureLen, (const unsigned char*)data, dataLen);
    if (verifyStatus != 1)
    {
        GFN_CC_LOG("Digest verification failed: %d\n", verifyStatus);
        goto end;
    }

    result = true;

end:
    EVP_MD_CTX_free(digestVerificationCtx);

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

    char *firstDot = NULL;
    char *secondDot = NULL;

    STACK_OF(X509) *certChain = NULL;

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


    if (!CreateX509CertificateChain(x5cCerts, numX5cCerts, s_RootPublicCert, &certChain))
    {
        GFN_CC_LOG("Failed to create certificate stack\n");
        goto end;
    }

    if (!VerifyX509CertificateChain(certChain))
    {
        GFN_CC_LOG("Failed to validate certificate chain\n");
        goto end;
    }

    // verify signature of (header + "." + payload)
    dataLen = secondDot - jwt;
    data = GFN_CC_MALLOC(dataLen);
    if (data == NULL)
    {
        GFN_CC_LOG("Failed to allocate memory for data object\n");
        goto end;
    }
    memcpy(data, jwt, dataLen);

    if (!VerifySignature(data, dataLen, decodedSignature, decodedSignatureLen, certChain))
    {
        GFN_CC_LOG("Failed to verify signature\n");
        goto end;
    }

    result = true;

end:
    // Free x5c certificates
    if (numX5cCerts > 0)
    {
        for (size_t i = 0; i < numX5cCerts; i++)
        {
            if (x5cCerts[i] != NULL)
            {
                GFN_CC_FREE(x5cCerts[i]);
            }
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

    sk_X509_pop_free(certChain, X509_free);

    return result;
}
