set(PROJECT_NAME video_capture)
add_library(${PROJECT_NAME} SHARED src/video_capture.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_VIDEO_CAPTURE)
target_link_directories(${PROJECT_NAME} PRIVATE "../ffmpeg/${CMAKE_SYSTEM_NAME}/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE avutil avformat avfilter avdevice avcodec swresample swscale)
target_include_directories(${PROJECT_NAME}
    PRIVATE "../ffmpeg/include"
    PRIVATE include/${PROJECT_NAME}
    PUBLIC include
)
