#include "GfnUtils.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

unsigned int GfnGetBase64EncodedLength(unsigned int srcBufSize)
{
    // Check for overflow
    if (srcBufSize > UINT_MAX / 4)
    {
        return (unsigned int)-1;
    }
    // Calculate base64 encoded length: 4 chars per 3 bytes, rounded up
    // Add 1 for null terminator
    return ((srcBufSize + 2) / 3) * 4 + 1;
}

int GfnBase64Encode(const unsigned char* input, unsigned int input_len, char* output, unsigned int output_size)
{
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int output_len = 0;
    unsigned int required_size;
    unsigned int i;
    unsigned int val;
    
    // Validate input parameters
    if (input == NULL || output == NULL || output_size == 0) {
        return -1;
    }
    
    // Calculate required output size (4 chars per 3 bytes, rounded up)
    required_size = ((input_len + 2) / 3) * 4;
    if (required_size >= output_size) {
        return -1;
    }
    
    // Encode the data
    for (i = 0; i < input_len; i += 3) {
        val = ((unsigned char)input[i] << 16);
        if (i + 1 < input_len) val |= ((unsigned char)input[i + 1] << 8);
        if (i + 2 < input_len) val |= (unsigned char)input[i + 2];

        output[output_len++] = base64_chars[(val >> 18) & 63];
        output[output_len++] = base64_chars[(val >> 12) & 63];
        output[output_len++] = base64_chars[(val >> 6) & 63];
        output[output_len++] = base64_chars[val & 63];
    }
    
    // Add padding if needed
    while (output_len % 4 != 0) {
        output[output_len++] = '=';
    }
    
    // Null terminate
    output[output_len] = '\0';
    
    return (int)output_len;
} 