cmake_minimum_required(VERSION 3.11)
project(GfnSdkPreWarmSample)

set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
)

add_executable(GfnSdkPreWarmSample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(GfnSdkPreWarmSample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(GfnSdkPreWarmSample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(GfnSdkPreWarmSample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(GfnSdkPreWarmSample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(GfnSdkPreWarmSample PRIVATE FEATURECONFIG_HEADER)

if (WIN32)
    set_target_properties(GfnSdkPreWarmSample PROPERTIES LINK_FLAGS "/ignore:4099")
endif (WIN32)

install(TARGETS GfnSdkPreWarmSample
    DESTINATION ./
    COMPONENT sdk_prewarmapp
)
