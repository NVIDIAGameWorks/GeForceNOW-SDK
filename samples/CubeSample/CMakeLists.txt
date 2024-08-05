project(CubeSample)

set(CUBE_SAMPLE_APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GfnSdkInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GfnSdkInterface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cube/cube.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cube/cube.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${GFN_SDK_DIST_DIR}/include/GfnRuntimeSdk_Wrapper.c
)

if (WIN32)
  list(APPEND CUBE_SAMPLE_APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc
    ${GFN_SDK_DIST_DIR}/include/GfnSdk_SecureLoadLibrary.c
  )
endif()

add_executable(CubeSample ${CUBE_SAMPLE_APP_SOURCES})

if (WIN32)
  target_compile_definitions(CubeSample
    PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
        _USE_MATH_DEFINES
        VK_NO_PROTOTYPES
        _CRT_SECURE_NO_WARNINGS
  )
else()
  target_compile_definitions(CubeSample
    PRIVATE
        VK_USE_PLATFORM_XLIB_KHR
        VK_NO_PROTOTYPES
  )

  target_link_libraries(CubeSample
    PUBLIC
        m
        X11
        dl
  )
endif()

target_include_directories(CubeSample
    PUBLIC
        ${GFN_SDK_DIST_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cube
        ${CMAKE_CURRENT_SOURCE_DIR}/cube/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/cube/Include/Volk
)

set_target_properties(CubeSample PROPERTIES OUTPUT_NAME "CubeSample")
set_target_properties(CubeSample PROPERTIES FOLDER "Dist/Samples")
