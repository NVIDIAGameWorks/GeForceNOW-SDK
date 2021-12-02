set(GFN_SDK_SAMPLE_SOURCES
    ${GFN_SDK_RUNTIME_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.h
)

add_executable(SDKDllDirectRefSample ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(SDKDllDirectRefSample PROPERTIES FOLDER "dist/samples")
set_target_properties(SDKDllDirectRefSample PROPERTIES LINK_FLAGS "/ignore:4099")

target_include_directories(SDKDllDirectRefSample PRIVATE ${GFN_SDK_DIST_DIR}/include)

# Set the configuration-specific binary output directory.
set(TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")

# Copy SDK dll to output directory.
add_custom_command(TARGET SDKDllDirectRefSample
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GFN_SDK_LIB_DIR}/GfnRuntimeSdk.dll"
        "${TARGET_OUT_DIR}"
    COMMENT "Copying SDK DLL to build..."
)
