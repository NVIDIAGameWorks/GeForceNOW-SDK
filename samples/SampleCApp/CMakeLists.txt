set(GFN_SDK_SAMPLE_SOURCES
    ${GFN_SDK_RUNTIME_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.h
)

add_executable(SampleCApp ${GFN_SDK_SAMPLE_SOURCES})
set_target_properties(SampleCApp PROPERTIES FOLDER "dist/samples")
set_target_properties(SampleCApp PROPERTIES LINK_FLAGS "/ignore:4099")

target_include_directories(SampleCApp
    PRIVATE
        ${GFN_SDK_DIST_DIR}/include
)
