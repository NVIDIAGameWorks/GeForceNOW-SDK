if (WIN32)
    set(GFN_SDK_SAMPLE_COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckAppAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform/Win/GfnCloudCheckUtils.c
    )
elseif (LINUX)
    set(GFN_SDK_SAMPLE_COMMON_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckAppAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GfnCloudCheckUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform/Posix/GfnCloudCheckUtils.c
    )
endif ()

add_library(SampleCommonUtils STATIC ${GFN_SDK_SAMPLE_COMMON_SOURCES})
set_target_properties(SampleCommonUtils PROPERTIES FOLDER "Dist/Samples")
target_include_directories(SampleCommonUtils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
