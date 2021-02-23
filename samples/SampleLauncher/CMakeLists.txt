# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

# OVERVIEW
#
# CMake is a cross-platform open-source build system that can generate project
# files in many different formats. It can be downloaded from
# http://www.cmake.org or installed via a platform package manager.
#
# CMake-generated project formats that have been tested with this CEF binary
# distribution include:
#
# Windows:    Ninja, Visual Studio 2010+
#
# Ninja is a cross-platform open-source tool for running fast builds using
# pre-installed platform toolchains (GNU, clang, Xcode or MSVC). It can be
# downloaded from http://martine.github.io/ninja/ or installed via a platform
# package manager.
#
# CMAKE STRUCTURE
#
# This CEF binary distribution includes the following CMake files:
#
# CMakeLists.txt              Bootstrap that sets up the CMake environment.
# cmake/*.cmake               CEF configuration files shared by all targets.
# libcef_dll/CMakeLists.txt   Defines the libcef_dll_wrapper target.
# tests/*/CMakeLists.txt      Defines the test application target.
#
# See the "TODO:" comments below for guidance on how to integrate this CEF
# binary distribution into a new or existing CMake project.
#
# BUILD REQUIREMENTS
#
# The below requirements must be met to build this CEF binary distribution.
#
# - CMake version 2.8.12.1 or newer.
#
# - Windows requirements:
#   Visual Studio 2010 or newer building on Windows 7 or newer. Visual Studio
#   2015 Update 3 and Windows 10 64-bit are recommended.
#
# BUILD EXAMPLES
#
# The below commands will generate project files and create a Debug build of all
# CEF targets using CMake and the platform toolchain.
#
# Start by creating and entering the CMake build output directory:
# > cd path/to/cef_binary_*
# > mkdir build && cd build
#
#   Using Ninja:
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
#   Using Ninja:
#     > cmake -G "Ninja" -DPROJECT_ARCH="x86_64" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a Windows build using a 32-bit CEF binary distribution:
#   Using the Visual Studio 2015 IDE:
#     > cmake -G "Visual Studio 14" ..
#     Open build\cef.sln in Visual Studio and select Build > Build Solution.
#
#   Using Ninja with Visual Studio 2015 command-line tools:
#     (this path may be different depending on your Visual Studio installation)
#     > "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a Windows build using a 64-bit CEF binary distribution:
#   Using the Visual Studio 2015 IDE:
#     > cmake -G "Visual Studio 14 Win64" ..
#     Open build\cef.sln in Visual Studio and select Build > Build Solution.
#
#   Using Ninja with Visual Studio 2015 command-line tools:
#     (this path may be different depending on your Visual Studio installation)
#     > "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple

#
# Global setup.
#

cmake_minimum_required(VERSION 3.11)
project(GfnSdkSampleLauncher)

set(USE_SANDBOX OFF CACHE BOOL "")

#
# Get CEF, if needed
#
cmake_policy(SET CMP0074 NEW)
set(libcef_download_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/libcef_download-src")
set(CEF_SOURCE_FILE "${CMAKE_BINARY_DIR}/_deps/libcef_source-url.txt")

# Override CEF location when requested
if (EXISTS ${SAMPLES_CEF_PATH})
    message("Overriding libcef_download_SOURCE_DIR with SAMPLES_CEF_PATH=${SAMPLES_CEF_PATH}")
    set(libcef_download_SOURCE_DIR ${SAMPLES_CEF_PATH})
else ()
    # Generate libcef URL if not set by cache
    set (LIBCEF_URL ${SAMPLES_CEF_URL})
    if (NOT LIBCEF_URL)
        if (WIN32)
            set(OS_STRING windows)
        elseif (APPLE)
            message( FATAL_ERROR "MacOS builds are not supported at this time." )
        elseif (LINUX)
            message( FATAL_ERROR "Linux builds are not supported at this time." )
        endif ()

        if (SAMPLES_ARCH STREQUAL "32")
            set(PROJECT_ARCH "x86")
        else ()
            set(PROJECT_ARCH "x86_64")
        endif ()

        set(CURRENT_LIBCEF_URL "https://cef-builds.spotifycdn.com/cef_binary_3.3626.1894.g90eb8cc_${OS_STRING}${SAMPLES_ARCH}.tar.bz2")
    endif ()
    set (EXISTING_CEF_SOURCE_URL "This is not a valid version string")
    if (EXISTS ${CEF_SOURCE_FILE})
        file(READ ${CEF_SOURCE_FILE} EXISTING_CEF_SOURCE_URL)
    endif()
    if ((NOT EXISTS ${libcef_download_SOURCE_DIR}) OR (NOT EXISTS ${CEF_SOURCE_FILE}) OR (NOT EXISTING_CEF_SOURCE_URL STREQUAL CURRENT_LIBCEF_URL))
        # Fetch libcef
        include(FetchContent)
        FetchContent_Declare(libcef_download
        URL ${CURRENT_LIBCEF_URL}
        )
        message("Please wait, downloading libcef from ${CURRENT_LIBCEF_URL} to ${libcef_download_SOURCE_DIR}")
        message("In order to avoid this, download CEF yourself and point cache variable SAMPLES_CEF_PATH to the downloaded location")
        FetchContent_Populate(libcef_download)
        file(WRITE ${CEF_SOURCE_FILE} ${CURRENT_LIBCEF_URL})
    endif()
endif ()

#
# Include cef package
#

set(CEF_ROOT "${libcef_download_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

# CEF isn't careful about Release vs RelWithDebInfo/MinSizeRel
string(REPLACE "$<CONFIGURATION>" "$<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>" CEF_BINARY_DIR ${CEF_BINARY_DIR})

#
# Define CEF-based targets.
#

# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
set_target_properties(libcef_dll_wrapper PROPERTIES FOLDER Helpers)

# Include application targets.
# Comes from the <target>/CMakeLists.txt file in the current directory.
add_subdirectory(src)

# Display configuration settings.
PRINT_CEF_CONFIG()
