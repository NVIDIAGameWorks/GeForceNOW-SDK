cmake_minimum_required(VERSION 3.11)
project(GfnSdkCloudCheckAPISample)

set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
)

add_executable(GfnSdkCloudCheckAPISample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(GfnSdkCloudCheckAPISample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(GfnSdkCloudCheckAPISample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(GfnSdkCloudCheckAPISample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(GfnSdkCloudCheckAPISample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(GfnSdkCloudCheckAPISample PRIVATE FEATURECONFIG_HEADER)

if (WIN32)
    set_target_properties(GfnSdkCloudCheckAPISample PROPERTIES LINK_FLAGS "/ignore:4099")
endif (WIN32)

install(TARGETS GfnSdkCloudCheckAPISample
    DESTINATION ./
    COMPONENT sdk_cloudcheckapp
)
