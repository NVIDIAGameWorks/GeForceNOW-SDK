set(GFN_RUNTIME_SDK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.h
)

add_executable(SampleCApp ${GFN_RUNTIME_SDK_SOURCES})
set_target_properties(SampleCApp PROPERTIES FOLDER "dist/samples")
set_target_properties(SampleCApp PROPERTIES LINK_FLAGS "/ignore:4099")

target_include_directories(SampleCApp
    PRIVATE
        ${GFN_RUNTIME_SDK_DIR}/include
)

target_link_libraries(SampleCApp
    PRIVATE
        ${GFN_RUNTIME_SDK_LIB_DIR}/${GFN_RUNTIME_LIB}
)
