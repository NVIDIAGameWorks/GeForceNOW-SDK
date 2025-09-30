cmake_minimum_required(VERSION 3.11)
project(GfnSdkPartnerDataAPISample)

set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
)

add_executable(GfnSdkPartnerDataAPISample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(GfnSdkPartnerDataAPISample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(GfnSdkPartnerDataAPISample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(GfnSdkPartnerDataAPISample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(GfnSdkPartnerDataAPISample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(GfnSdkPartnerDataAPISample PRIVATE FEATURECONFIG_HEADER)

if (WIN32)
    set_target_properties(GfnSdkPartnerDataAPISample PROPERTIES LINK_FLAGS "/ignore:4099")
endif (WIN32)

install(TARGETS GfnSdkPartnerDataAPISample
    DESTINATION ./
    COMPONENT sdk_partnerdataapp
)
