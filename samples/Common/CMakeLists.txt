set(UTILS_LIB_TARGET GfnSdkSampleCommonUtils)
add_library(${UTILS_LIB_TARGET} STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckAppAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckUtils.h
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/Platform/Posix/GfnCloudCheckUtils.c>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/Platform/Win/GfnCloudCheckUtils.c>
)
set_target_properties(${UTILS_LIB_TARGET} PROPERTIES FOLDER "Dist/Samples")
set_target_properties(${UTILS_LIB_TARGET} PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckUtils.h")
target_include_directories(${UTILS_LIB_TARGET} PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
elseif (LINUX)
    target_compile_options(${UTILS_LIB_TARGET} 
        PRIVATE
            ${STRICT_WARNINGS}
    )
endif()

if (LINUX AND BUILD_INTERNAL_OPENSSL)
    include(ExternalProject)

    set(OpenSSL_RELEASE_URL "https://github.com/openssl/openssl/releases/download/openssl-3.3.1/openssl-3.3.1.tar.gz")
    set(OpenSSL_RELEASE_SHA256 "777cd596284c883375a2a7a11bf5d2786fc5413255efab20c50d6ffe6d020b7e") # https://github.com/openssl/openssl/releases/download/openssl-3.3.1/openssl-3.3.1.tar.gz.sha256

    set(OpenSSL_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/OpenSSL_download-src")
    set(OpenSSL_BUILD_DIR "${CMAKE_BINARY_DIR}/_deps/OpenSSL_download-build")
    set(OpenSSL_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(OpenSSL_INSTALLED_CRYPTO_LIBRARY "${OpenSSL_INSTALL_DIR}/lib64/libcrypto.a")

    if(CMAKE_VERSION VERSION_GREATER "3.23")
        # Should use DOWNLOAD_EXTRACT_TIMESTAMP option for ExternalProject_Add when minimum cmake version is raised
        cmake_policy(SET CMP0135 NEW)
    endif ()

    # Note: we're using ExternalProeject_Add over FetchContent since FetchContent
    # requires the incoming dependency to be a cmake project and OpenSSL doesn't use it as its build system
    ExternalProject_Add(
        OpenSSL_External
        URL ${OpenSSL_RELEASE_URL}
        URL_HASH SHA256=${OpenSSL_RELEASE_SHA256}
        SOURCE_DIR ${OpenSSL_SOURCE_DIR}
        BINARY_DIR ${OpenSSL_BUILD_DIR}
        USES_TERMINAL_DOWNLOAD true
        USES_TERMINAL_BUILD true
        USES_TERMINAL_INSTALL true
        BUILD_BYPRODUCTS "${OpenSSL_INSTALLED_CRYPTO_LIBRARY}"   # no INSTALL_BYPRODUCTS for cmake < 3.26
        CONFIGURE_COMMAND "${OpenSSL_SOURCE_DIR}/Configure"
                            "--prefix=${OpenSSL_INSTALL_DIR}"
                            "no-shared"
                            "no-apps"
                            "no-autoload-config"
                            "no-deprecated"
                            "no-docs"
                            "no-http"
                            "no-legacy"
                            "no-srp"
                            "no-srtp"
                            "no-ssl-trace"
                            "no-tests"
                            "no-quic"
                            "no-ui-console"
                            "no-tls"
                            "no-dtls"
                            "no-ssl"
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        INSTALL_DIR ${OpenSSL_INSTALL_DIR}
    )
    
    add_dependencies(${UTILS_LIB_TARGET} OpenSSL_External)
    
    target_include_directories(${UTILS_LIB_TARGET} PRIVATE "${OpenSSL_INSTALL_DIR}/include")
    target_link_libraries(${UTILS_LIB_TARGET} PUBLIC "${OpenSSL_INSTALLED_CRYPTO_LIBRARY}")
elseif(LINUX AND NOT BUILD_INTERNAL_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${UTILS_LIB_TARGET} PUBLIC OpenSSL::Crypto)
endif()

if (LINUX)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    target_link_libraries(${UTILS_LIB_TARGET} PUBLIC
        Threads::Threads
    )
endif ()
