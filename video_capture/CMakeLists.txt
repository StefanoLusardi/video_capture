message("")
message(":::::::::::::::::::::::::")
message("::VideoCapture:: Library")
message("")

set(PROJECT_NAME video_capture)
add_library(${PROJECT_NAME} SHARED src/video_capture.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_VIDEO_CAPTURE)

set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/ffmpeg")
target_link_directories(${PROJECT_NAME} PRIVATE "${FFMPEG_DIR}/${CMAKE_SYSTEM_NAME}/lib")

set(FFMPEG_LIBS avutil avformat avfilter avdevice avcodec swresample swscale)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBS})

target_include_directories(${PROJECT_NAME}
    PRIVATE "${FFMPEG_DIR}/include"
    PRIVATE include/${PROJECT_NAME}
    PUBLIC include)

if(WIN32)
    message(STATUS "Copy FFMPEG binaries")
    file(COPY "${FFMPEG_DIR}/${CMAKE_SYSTEM_NAME}/bin/" 
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING PATTERN "*.dll*")
endif()

if(UNIX)
    message(STATUS "Create symlinks for FFMPEG libs")
    set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/${CMAKE_SYSTEM_NAME}/lib")
    FILE(GLOB LIBS "${FFMPEG_LIB_DIR}/lib*.so.*")
    foreach(LIB ${LIBS})
        string(REGEX MATCH ".*.so" LIB_SYMLINK ${LIB})
        message("lib:     ${LIB}")
        message("symlink: ${LIB_SYMLINK}")
        execute_process(COMMAND ln -sf "${LIB}" "${LIB_SYMLINK}")
    endforeach()

    message(STATUS "Copy FFMPEG binaries")
    file(COPY "${FFMPEG_LIB_DIR}/"
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING PATTERN "*.so*")
endif()