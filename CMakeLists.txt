cmake_minimum_required (VERSION 3.19.0)

# Define LINUX for compatibility with earlier versions of cmake
if(${CMAKE_MAJOR_VERSION} LESS_EQUAL 3 AND ${CMAKE_MINOR_VERSION} LESS 25 AND UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

project (GFNSDK)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD_ARCH x64)
    set(SAMPLES_ARCH "64" CACHE STRING "Build architecture for sample apps")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BUILD_ARCH x86)
    set(SAMPLES_ARCH "32" CACHE STRING "Build architecture for sample apps")
endif()

if (WIN32)
    # To enable usage of CMAKE_MSVC_RUNTIME_LIBRARY
    cmake_policy(SET CMP0091 NEW)
    # Statically link the CRT to avoid needing to install CRT debug binaries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

##### Cache Configuration #####
set(GFN_SDK_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Location of the GFN SDK root")

if(NOT GFN_SDK_LIB_DIR)
    set(GFN_SDK_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_ARCH}
        CACHE PATH "Location of the GFN SDK library binaries"
    )
endif()

set(SAMPLE_LAUNCHER_INSTALL_ROOT "" CACHE STRING "(Optional) location to install sample launcher with cmake install command")
set(SAMPLE_CLOUDCHECKAPI_INSTALL_ROOT "" CACHE STRING "(Optional) location to install Cloud Check API sample with cmake install command")
set(SAMPLES_CEF_URL "" CACHE STRING "(Optional) URL to use to download libcef for launcher sample")
set(SAMPLES_CEF_PATH "" CACHE STRING "(Optional) Path to find pre-downloaded CEF")

set_property(CACHE SAMPLES_ARCH PROPERTY STRINGS 64 32)
option(BUILD_SAMPLES "Build the GFN SDK samples" ON)
set(AVAILABLE_SAMPLES CGameAPISample CloudCheckAPI CubeSample OpenClientBrowser PartnerDataAPI PreWarmSample SDKDllDirectRefSample SampleLauncher)
set(BUILD_SAMPLES_LIST "${AVAILABLE_SAMPLES}" CACHE STRING "List of GFN SDK samples to build (e.g. 'CGameAPISample;CloudCheckAPI)")
if (LINUX)
    # If the option is set to `OFF` then OpenSSL dependency can be provided by the user instead
    # of downloading it from the internet.
    #
    # The `find_package(OpenSSL REQUIRED)` call will be made, and it's up to the user how to
    # make this call succeed. E.g. if OpenSSL was built and installed elsewhere on the machine,
    # then normally it's enough to set `OPENSSL_ROOT_DIR` variable as a hint for CMake.
    #
    # See https://cmake.org/cmake/help/latest/module/FindOpenSSL.html for more details.
    option(BUILD_INTERNAL_OPENSSL "Download and build OpenSSL internally within the samples" ON)
endif ()

###############################

# For the GfnSdk distribution, GfnRuntimeSdk.dll is precompiled
if (NOT TARGET GfnRuntime)
    add_library(GfnRuntime MODULE IMPORTED)
    if (WIN32)
        set_target_properties(GfnRuntime PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/${BUILD_ARCH}/GfnRuntimeSdk.dll
        )
    elseif (LINUX)
        set_target_properties(GfnRuntime PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${BUILD_ARCH}/GfnRuntimeSdk.so
        )
    else ()
        message(FATAL_ERROR "Unsupported platform")
    endif ()
endif ()

add_library(GfnSdkWrapper STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.c
)
set(GfnSdkWrapper_Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_CAPI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk.h
)
set_target_properties(GfnSdkWrapper PROPERTIES
    PUBLIC_HEADER "${GfnSdkWrapper_Headers}"
)

if (WIN32)
    add_library(GfnSdkSecureLoadLibrary STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.c
    )
    target_link_libraries(GfnSdkWrapper PRIVATE GfnSdkSecureLoadLibrary)
elseif (LINUX)
    set(STRICT_WARNINGS
        -Werror
        -Wformat
        -Wreturn-type
        -Wswitch
        -Wunused-local-typedefs
        -Wparentheses
        -Wpointer-arith
        -Wall
        -Wextra
        -Wno-sign-compare
        -Wno-unused-parameter
        -Wmissing-declarations
        -Wimplicit
        -Wno-format-zero-length
        -Wdeclaration-after-statement
        -Wstrict-prototypes
        -Wmissing-prototypes
    )
    target_link_libraries(GfnSdkWrapper PUBLIC ${CMAKE_DL_LIBS})
    target_compile_options(GfnSdkWrapper
        PUBLIC
            -fPIC
        PRIVATE
            ${STRICT_WARNINGS}
    )
endif ()

target_include_directories(GfnSdkWrapper PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (BUILD_SAMPLES)
    # Prerequisite library for a number of samples
    add_subdirectory(samples/Common)

    foreach(SAMPLE ${BUILD_SAMPLES_LIST})
        if (SAMPLE IN_LIST AVAILABLE_SAMPLES)
            message(STATUS "Adding sample to the build: ${SAMPLE}")
            add_subdirectory("samples/${SAMPLE}")
        else ()
            message(FATAL_ERROR "Skipping unknown sample: ${SAMPLE}")
        endif ()
    endforeach()
endif ()
