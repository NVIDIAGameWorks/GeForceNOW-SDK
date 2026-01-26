#ifndef __GFN_UTILS_H__
#define __GFN_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

    // Description
    // Calculates the required buffer size for base64 encoding.
    // This function calculates the exact buffer size needed to store the base64-encoded string.
    //
    // Parameters
    // srcBufSize The size of the source binary data.
    //
    // Return Values
    // The required buffer size (including null terminator) on success, (unsigned int)-1 on failure.
    unsigned int GfnGetBase64EncodedLength(unsigned int srcBufSize);

    // Description
    // Encodes binary data to base64 string.
    // This function converts binary data to a base64-encoded string for safe transmission.
    //
    // Parameters
    // input The binary data to encode.
    // input_len The length of the input data.
    // output The output buffer where the base64 string will be stored.
    // output_size The size of the output buffer.
    //
    // Return Values
    // The number of characters written to the output buffer (excluding null terminator) on success, -1 on failure.
    int GfnBase64Encode(const unsigned char* input, unsigned int input_len, char* output, unsigned int output_size);

#ifdef __cplusplus
}
#endif

#endif // __GFN_UTILS_H__
