set(GFN_SDK_SAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.h
)

add_executable(SDKDllDirectRefSample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(SDKDllDirectRefSample PROPERTIES FOLDER "Dist/Samples")

target_include_directories(SDKDllDirectRefSample PRIVATE ${GFN_SDK_DIST_DIR}/include)

if (WIN32)
    target_link_libraries(SDKDllDirectRefSample PRIVATE GfnSdkSecureLoadLibrary)
    set_target_properties(SDKDllDirectRefSample PROPERTIES LINK_FLAGS "/ignore:4099")
    set(_gfn_runtime_shared_lib "${CMAKE_CURRENT_BINARY_DIR}/GfnRuntimeSdk.dll")
elseif (LINUX)
    target_link_libraries(SDKDllDirectRefSample PRIVATE ${CMAKE_DL_LIBS})
    set(_gfn_runtime_shared_lib "${CMAKE_CURRENT_BINARY_DIR}/GfnRuntimeSdk.so")
endif (WIN32)

# Copy SDK shared library to output directory to bring the sample up to date
add_custom_command(
    OUTPUT "${_gfn_runtime_shared_lib}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GfnRuntime> ${_gfn_runtime_shared_lib}
    DEPENDS GfnRuntime
)

target_sources(SDKDllDirectRefSample PRIVATE "${_gfn_runtime_shared_lib}")
