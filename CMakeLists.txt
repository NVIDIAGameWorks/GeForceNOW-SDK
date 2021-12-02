cmake_minimum_required (VERSION 3.2.0)
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

##### Cache Configuration #####
set(GFN_SDK_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Location of the GFN SDK root")

if(NOT GFN_SDK_LIB_DIR)
    set(GFN_SDK_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_ARCH}
        CACHE PATH "Location of the GFN SDK library binaries"
    )
endif()

set(SAMPLES_INSTALL_ROOT "" CACHE STRING "(Optional) location to install sample launcher with cmake install command")
set(SAMPLES_CEF_URL "" CACHE STRING "(Optional) URL to use to download libcef for launcher sample")
set(SAMPLES_CEF_PATH "" CACHE STRING "(Optional) Path to find pre-downloaded CEF")

set_property(CACHE SAMPLES_ARCH PROPERTY STRINGS 64 32)
if (WIN32)
    set(USE_STATIC_CRT ON CACHE BOOL "(Windows) Enable to statically link against the CRT")
endif ()
option(BUILD_SAMPLES "Build the GFN SDK samples" ON)
###############################

# Force MSVC static runtime library for each configuration
# Replace the strings instead of appending /MT(d), because overriding gives a compile error
# This handles both /MDd -> /MTd, and /MD -> /MT
foreach(i CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO)
    string(REGEX REPLACE "/MD" "/MT" ${i} "${${i}}")
endforeach()
set(MSVC_RUNTIME "MT")

set(GFN_SDK_RUNTIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_CAPI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.c
)

set(GFN_SDK_COMMON_SOURCES
    ${GENERATED_GFN_SDK_H}
)

add_custom_target(Dist SOURCES ${GFN_SDK_RUNTIME_SOURCES} ${GFN_SDK_COMMON_SOURCES})

if (BUILD_SAMPLES)
    add_subdirectory(samples/CGameAPISample)
    add_subdirectory(samples/SDKDllDirectRefSample)
    add_subdirectory(samples/SampleService)

    if (USE_STATIC_CRT)
        add_subdirectory(samples/SampleLauncher)
    else()
        message(WARNING "Sample Launcher will NOT be configured since it requires static CRT linkage.")
    endif()
endif ()
