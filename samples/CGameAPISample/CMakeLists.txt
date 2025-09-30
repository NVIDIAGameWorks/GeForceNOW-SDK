set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.h
)

add_executable(CGameAPISample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(CGameAPISample PROPERTIES FOLDER "Dist/Samples")

target_link_libraries(CGameAPISample PRIVATE GfnSdkWrapper GfnSdkSampleCommonUtils)
target_include_directories(CGameAPISample PRIVATE ${GFN_SDK_DIST_DIR}/include)
target_include_directories(CGameAPISample PRIVATE ${GFN_SDK_DIST_DIR}/samples/Common)
target_compile_definitions(CGameAPISample PRIVATE FEATURECONFIG_HEADER)

if (WIN32)
    set_target_properties(CGameAPISample PROPERTIES LINK_FLAGS "/ignore:4099")
endif (WIN32)
