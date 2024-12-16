# Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

if(OS_MACOSX)
  message(FATAL_ERROR "MacOS builds are not supported at this time.")
endif()

#
# Source files.
#

# Main executable sources.
set(EXAMPLE_SRCS
  ../minimal/main_minimal.cc
  client_impl.cc
  client_impl.h
  gfn_sdk_helper.cc
  gfn_sdk_helper.h
  )
set(EXAMPLE_SRCS_WINDOWS
  app_browser_win_impl.cc
  resource_util_win_impl.cc
  )
set(EXAMPLE_SRCS_LINUX
  app_browser_impl.cc
  resource_util_win_impl.cc
  )
APPEND_PLATFORM_SOURCES(EXAMPLE_SRCS)

if(OS_LINUX OR OS_WINDOWS)
  # On Windows and Linux the same executable is used for all processes.
  set(EXAMPLE_SRCS
    ${EXAMPLE_SRCS}
    ../minimal/app_other_minimal.cc
    app_renderer_impl.cc
    )
elseif(OS_MACOSX)
  # On macOS a separate helper executable is used for subprocesses.
  set(EXAMPLE_HELPER_SRCS
    ../minimal/app_other_minimal.cc
    ../minimal/process_helper_mac_minimal.cc
    app_renderer_impl.cc
    )
endif()

# Main executable resources.
set(EXAMPLE_RESOURCES_SRCS
  resources/gfn_sdk.html
  )
set(EXAMPLE_RESOURCES_SRCS_WINDOWS
  # Resources that embed "gfn_sdk.html" in the executable.
  resources/win/resource.h
  resources/win/resource.rc
  )
APPEND_PLATFORM_SOURCES(EXAMPLE_RESOURCES_SRCS)

if(OS_MACOSX OR OS_WINDOWS)
  # On macOS and Windows include the message_router and shared resources.
  set(EXAMPLE_RESOURCES_SRCS
    ${EXAMPLE_RESOURCES_SRCS}
    ${SHARED_RESOURCES_SRCS}
    )
endif()


#
# Shared configuration.
#

# Target executable names.
set(EXAMPLE_TARGET "GfnSdkSampleLauncher")
if(OS_MACOSX)
  set(EXAMPLE_HELPER_TARGET "GfnSdkSampleLauncher Helper")
  set(EXAMPLE_HELPER_OUTPUT_NAME "GfnSdkSampleLauncher Helper")
endif()


#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${EXAMPLE_TARGET} ${EXAMPLE_SRCS})
  SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(${EXAMPLE_TARGET})
  add_dependencies(${EXAMPLE_TARGET} shared)
  target_link_libraries(${EXAMPLE_TARGET} shared GfnSdkWrapper)
  target_link_libraries(${EXAMPLE_TARGET} shared GfnSdkSampleCommonUtils)

  target_include_directories(${EXAMPLE_TARGET} PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)

  # Copy SDK .so to output directory to bring the sample up to date
  set(_gfn_runtime_so "${CMAKE_BINARY_DIR}/SampleLauncher/GfnRuntimeSdk.so")
  add_custom_command(
      OUTPUT "${_gfn_runtime_so}"
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GfnRuntime> ${_gfn_runtime_so}
  )
  target_sources(${EXAMPLE_TARGET} PRIVATE "${_gfn_runtime_so}")

  # Copy resource files to the target output directory.
  COPY_FILES("${EXAMPLE_TARGET}" "${EXAMPLE_RESOURCES_SRCS}" "${CMAKE_CURRENT_SOURCE_DIR}" "${EXAMPLE_TARGET_OUT_DIR}/${EXAMPLE_TARGET}_files")
endif()


#
# Mac OS X configuration.
#

if(OS_MACOSX)
  # Create source groups for Xcode.
  SET_EXAMPLE_SOURCE_GROUPS("${EXAMPLE_SRCS}")
  SET_EXAMPLE_SOURCE_GROUPS("${EXAMPLE_HELPER_SRCS}")

  # Output paths for the app bundles.
  set(EXAMPLE_APP "${EXAMPLE_TARGET_OUT_DIR}/${EXAMPLE_TARGET}.app")
  set(EXAMPLE_HELPER_APP "${EXAMPLE_TARGET_OUT_DIR}/${EXAMPLE_HELPER_OUTPUT_NAME}.app")

  # Variable referenced from Info.plist files.
  set(PRODUCT_NAME "${EXAMPLE_TARGET}")

  # Helper executable target.
  add_executable(${EXAMPLE_HELPER_TARGET} MACOSX_BUNDLE ${EXAMPLE_HELPER_SRCS})
  SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(${EXAMPLE_HELPER_TARGET})
  add_dependencies(${EXAMPLE_HELPER_TARGET} shared_helper)
  target_link_libraries(${EXAMPLE_HELPER_TARGET} shared_helper)
  set_target_properties(${EXAMPLE_HELPER_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_HELPER_INFO_PLIST}"
    OUTPUT_NAME ${EXAMPLE_HELPER_OUTPUT_NAME}
    )

  # Main executable target.
  add_executable(${EXAMPLE_TARGET} MACOSX_BUNDLE ${EXAMPLE_SRCS} ${EXAMPLE_RESOURCES_SRCS})
  SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(${EXAMPLE_TARGET})
  add_dependencies(${EXAMPLE_TARGET} shared "${EXAMPLE_HELPER_TARGET}")
  target_link_libraries(${EXAMPLE_TARGET} shared)
  set_target_properties(${EXAMPLE_TARGET} PROPERTIES
    RESOURCE "${EXAMPLE_RESOURCES_SRCS}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_INFO_PLIST}"
    )

  # Copy files into the main app bundle.
  add_custom_command(
    TARGET ${EXAMPLE_TARGET}
    POST_BUILD
    # Copy the helper app bundle into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${EXAMPLE_HELPER_APP}"
            "${EXAMPLE_APP}/Contents/Frameworks/${EXAMPLE_HELPER_OUTPUT_NAME}.app"
    # Copy the CEF framework into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${EXAMPLE_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    VERBATIM
    )

  if(NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
    # Manually process and copy over resource files.
    # The Xcode generator handles this via the set_target_properties RESOURCE directive.
    # Remove these prefixes from input file paths.
    set(PREFIXES
      "../shared/resources/mac/"
      "resources/"
      )
    COPY_MACOSX_RESOURCES("${EXAMPLE_RESOURCES_SRCS}" "${PREFIXES}" "${EXAMPLE_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${EXAMPLE_APP}")
  endif()
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
    # Add resources to the sources variable for convenience.
  set(EXAMPLE_SRCS
    ${EXAMPLE_SRCS}
    ${EXAMPLE_RESOURCES_SRCS}
    )

  # Create source groups for Visual Studio.
  SET_EXAMPLE_SOURCE_GROUPS("${EXAMPLE_SRCS}")

  # Executable target.
  add_executable(${EXAMPLE_TARGET} WIN32 ${EXAMPLE_SRCS})
  SET_EXAMPLE_EXECUTABLE_TARGET_PROPERTIES(${EXAMPLE_TARGET})
  add_dependencies(${EXAMPLE_TARGET} shared)
  target_link_libraries(${EXAMPLE_TARGET} shared GfnSdkWrapper)
  target_link_libraries(${EXAMPLE_TARGET} shared GfnSdkSampleCommonUtils)

  target_link_libraries(${EXAMPLE_TARGET}
    crypt32.lib
    Wldap32.lib
  )
  target_include_directories(${EXAMPLE_TARGET}
    PRIVATE
      ${GFN_SDK_DIST_DIR}/include
      ${CMAKE_BINARY_DIR}/../include
      ${GFN_SDK_DIST_DIR}/samples/Common
  )
  target_compile_features(${EXAMPLE_TARGET} PRIVATE cxx_std_17)
  set_source_files_properties(${EXAMPLE_TARGET} PROPERTIES COMPILE_FLAGS "/wd4996")
  target_compile_definitions(${EXAMPLE_TARGET} PRIVATE FEATURECONFIG_HEADER)

  # Copy SDK dll to output directory to bring the sample up to date
  set(_gfn_runtime_dll "${CMAKE_BINARY_DIR}/SampleLauncher/GfnRuntimeSdk.dll")
  add_custom_command(
      OUTPUT "${_gfn_runtime_dll}"
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GfnRuntime> ${_gfn_runtime_dll}
  )
  target_sources(${EXAMPLE_TARGET} PRIVATE "${_gfn_runtime_dll}")
endif()

set(EXAMPLE_INSTALL_FILES
  ${libcef_download_SOURCE_DIR}/Resources/cef.pak
  ${libcef_download_SOURCE_DIR}/Resources/icudtl.dat
)

if (WIN32)
  set(EXAMPLE_INSTALL_FILES_DEBUG
    ${libcef_download_SOURCE_DIR}/Debug/chrome_elf.dll
    ${libcef_download_SOURCE_DIR}/Debug/libcef.dll
    ${libcef_download_SOURCE_DIR}/Debug/natives_blob.bin
    ${libcef_download_SOURCE_DIR}/Debug/snapshot_blob.bin
    ${libcef_download_SOURCE_DIR}/Debug/v8_context_snapshot.bin
  )

  set(EXAMPLE_INSTALL_FILES_RELEASE
    ${libcef_download_SOURCE_DIR}/Release/chrome_elf.dll
    ${libcef_download_SOURCE_DIR}/Release/libcef.dll
    ${libcef_download_SOURCE_DIR}/Release/natives_blob.bin
    ${libcef_download_SOURCE_DIR}/Release/snapshot_blob.bin
    ${libcef_download_SOURCE_DIR}/Release/v8_context_snapshot.bin
  )
elseif (LINUX)
  set(EXAMPLE_INSTALL_FILES_DEBUG
    ${libcef_download_SOURCE_DIR}/Debug/libcef.so
    ${libcef_download_SOURCE_DIR}/Debug/libEGL.so
    ${libcef_download_SOURCE_DIR}/Debug/libGLESv2.so
    ${libcef_download_SOURCE_DIR}/Debug/natives_blob.bin
    ${libcef_download_SOURCE_DIR}/Debug/snapshot_blob.bin
    ${libcef_download_SOURCE_DIR}/Debug/v8_context_snapshot.bin
  )

  set(EXAMPLE_INSTALL_FILES_RELEASE
    ${libcef_download_SOURCE_DIR}/Release/libcef.so
    ${libcef_download_SOURCE_DIR}/Release/libEGL.so
    ${libcef_download_SOURCE_DIR}/Release/libGLESv2.so
    ${libcef_download_SOURCE_DIR}/Release/natives_blob.bin
    ${libcef_download_SOURCE_DIR}/Release/snapshot_blob.bin
    ${libcef_download_SOURCE_DIR}/Release/v8_context_snapshot.bin
  )
endif ()

set(SAMPLE_LAUNCHER_INSTALL_FILES         "${EXAMPLE_INSTALL_FILES}" CACHE INTERNAL "Example Install Files")
set(SAMPLE_LAUNCHER_INSTALL_FILES_DEBUG   "${EXAMPLE_INSTALL_FILES_DEBUG}" CACHE INTERNAL "Example Install Files Debug")
set(SAMPLE_LAUNCHER_INSTALL_FILES_RELEASE "${EXAMPLE_INSTALL_FILES_RELEASE}" CACHE INTERNAL "Example Install Files Release")
