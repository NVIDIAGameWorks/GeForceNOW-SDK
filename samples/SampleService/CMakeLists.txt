#sampleapplib static lib
set(SRV_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/deserialize_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/deserialize_iterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/lpc_pipe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/lpc_pipe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/memory_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/serialize_common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/serialize_iterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/traits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/transport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/command.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/port_name.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/status.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/GfnRuntimeSdk_Wrapper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/GfnSdk_SecureLoadLibrary.c
)
add_library(SampleServiceLib STATIC ${SRV_LIB})
set_source_files_properties(${SRV_LIB} PROPERTIES COMPILE_FLAGS "/wd4996")
set_source_files_properties(${SRV_LIB} PROPERTIES COMPILE_FLAGS "/wd4244")
target_include_directories(SampleServiceLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${GFN_SDK_DIST_DIR}/include
)
target_compile_features(SampleServiceLib PRIVATE cxx_std_17)
target_compile_definitions(SampleServiceLib 
    PRIVATE 
        GFN_SDK_WRAPPER_LOG 
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
set_target_properties(SampleServiceLib PROPERTIES FOLDER "dist/samples/GfnSdkSampleService/")
set_target_properties(SampleServiceLib PROPERTIES OUTPUT_NAME SampleServiceLib)

#Sample Service executable
set(SAMPLE_SRV_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/svc/instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/svc/instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/svc/service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/svc/service.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/svc/main.cpp
)
set_source_files_properties(${SAMPLE_SRV_SRCS} PROPERTIES COMPILE_FLAGS "/wd4996")
set_source_files_properties(${SAMPLE_SRV_SRCS} PROPERTIES COMPILE_FLAGS "/wd4244")
add_executable(SampleService ${SAMPLE_SRV_SRCS})
set_target_properties(SampleService PROPERTIES FOLDER "dist/samples/GfnSdkSampleService/")
target_link_libraries(SampleService PRIVATE SampleServiceLib)
target_include_directories(SampleService
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
)

 if (SAMPLES_INSTALL_ROOT)
    install(TARGETS SampleService
      DESTINATION ${PACK_SAMPLE_LAUNCHER_ROOT}
      COMPONENT sample_bins
    )
 endif ()
 if (SAMPLES_INSTALL_ROOT_VALVE)
 install(TARGETS SampleService
   DESTINATION ${PACK_SAMPLE_LAUNCHER_ROOT_VALVE}
   COMPONENT sample_bins_valve
 )
endif ()

