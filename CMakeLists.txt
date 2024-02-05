cmake_minimum_required (VERSION 3.19.0)
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
option(BUILD_SAMPLES "Build the GFN SDK samples" ON)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_CAPI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnRuntimeSdk_Wrapper.c
)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrtd")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt")
    # To enable usage of CMAKE_MSVC_RUNTIME_LIBRARY
    cmake_policy(SET CMP0091 NEW)
    # Statically link the CRT to avoid needing to install CRT debug binaries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_library(GfnSdkSecureLoadLibrary STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GfnSdk_SecureLoadLibrary.c
    )
    target_link_libraries(GfnSdkWrapper PRIVATE GfnSdkSecureLoadLibrary)
elseif (LINUX)
    target_link_libraries(GfnSdkWrapper PUBLIC ${CMAKE_DL_LIBS})
    target_compile_options(GfnSdkWrapper PUBLIC -fPIC)
endif ()

target_include_directories(GfnSdkWrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (BUILD_SAMPLES)
    add_subdirectory(samples/CGameAPISample)
    add_subdirectory(samples/SampleLauncher)
    add_subdirectory(samples/SDKDllDirectRefSample)
endif ()
