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
    set(_gfn_runtime_shared_lib "${CMAKE_CURRENT_BINARY_DIR}/GfnRuntimeSdk.dll")
elseif (LINUX)
    set(_gfn_runtime_shared_lib "${CMAKE_CURRENT_BINARY_DIR}/GfnRuntimeSdk.so")
endif (WIN32)

# Copy SDK shared library to output directory to bring the sample up to date
add_custom_command(
    OUTPUT "${_gfn_runtime_shared_lib}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GfnRuntime> ${_gfn_runtime_shared_lib}
    DEPENDS GfnRuntime
)
target_sources(GfnSdkPreWarmSample PRIVATE "${_gfn_runtime_shared_lib}")

install(TARGETS GfnSdkPreWarmSample
    DESTINATION ./
    COMPONENT sdk_prewarmapp
)
install(FILES $<TARGET_FILE:GfnRuntime>
    DESTINATION ./
    COMPONENT sdk_prewarmapp
)
