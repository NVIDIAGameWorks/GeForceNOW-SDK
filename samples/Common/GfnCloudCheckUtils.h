// This header file contains methods that facilitates CloudCheck API response validation.
// Game/application devs are free to use this implementation (*.h/*.c) files and integrate 
// within their build system.

#ifndef __GFN_CLOUD_CHECK_UTILS_H__
#define __GFN_CLOUD_CHECK_UTILS_H__


#ifdef __cplusplus
extern "C" {
#endif

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
    bool GfnCloudCheckGenerateNonce(char* nonce, unsigned int nonceSize);

    /**
     * @brief Validates attestation data received in CloudCheck API response represented as a JWT.
     *
     * This function performs a series of steps to validate the integrity of attestation data.
     *
     * @param attestationData The attestation data in JWT format.
     * @param nonce The nonce value to match with the value in the payload.
     *
     * @return true if the JWT response is valid, false otherwise.
     */
    bool GfnCloudCheckVerifyAttestationData(const char* jwt, const char* nonce, unsigned int nonceSize);

#ifdef __cplusplus
}
#endif

#endif //__GFN_CLOUD_CHECK_UTILS_H__
