cmake_minimum_required(VERSION 3.11)
project(GfnSdkOpenClientBrowserSample)

set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
)

add_executable(GfnSdkOpenClientBrowserSample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(GfnSdkOpenClientBrowserSample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(GfnSdkOpenClientBrowserSample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(GfnSdkOpenClientBrowserSample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(GfnSdkOpenClientBrowserSample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(GfnSdkOpenClientBrowserSample PRIVATE FEATURECONFIG_HEADER)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif()

if (WIN32)
    set_target_properties(GfnSdkOpenClientBrowserSample PROPERTIES LINK_FLAGS "/ignore:4099")
elseif (LINUX)
    find_package(X11 REQUIRED)
    target_include_directories(GfnSdkOpenClientBrowserSample PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(GfnSdkOpenClientBrowserSample PRIVATE ${X11_LIBRARIES})
endif (WIN32)

install(TARGETS GfnSdkOpenClientBrowserSample
    DESTINATION ./
    COMPONENT sdk_openclientbrowserapp
)
