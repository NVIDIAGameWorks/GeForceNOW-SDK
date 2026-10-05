cmake_minimum_required(VERSION 3.11)
project(GfnSdkSessionTerminationSample)

set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
)

add_executable(GfnSdkSessionTerminationSample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(GfnSdkSessionTerminationSample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(GfnSdkSessionTerminationSample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(GfnSdkSessionTerminationSample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(GfnSdkSessionTerminationSample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(GfnSdkSessionTerminationSample PRIVATE FEATURECONFIG_HEADER)

if (WIN32)
    set_target_properties(GfnSdkSessionTerminationSample PROPERTIES LINK_FLAGS "/ignore:4099")
endif (WIN32)

install(TARGETS GfnSdkSessionTerminationSample
    DESTINATION ./
    COMPONENT sdk_sessionterminationapp
)
