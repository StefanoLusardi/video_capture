message("")
message(":::::::::::::::::::::::::")
message("::VideoCapture:: Library")
message("")

set(PROJECT_NAME video_capture)
set(VCPP_FFMPEG_LIBS avutil avformat avdevice avcodec swscale)

set(VCPP_SOURCES 
    src/video_capture.cpp
    src/frame_sync.cpp
    src/hw_acceleration.hpp
    src/logger.hpp)

set(VCPP_HEADERS 
    include/video_capture/api.hpp
    include/video_capture/frame_queue.hpp
    include/video_capture/frame_sync.hpp
    include/video_capture/video_capture.hpp)

if(${VCPP_BUILD_SHARED} OR BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${VCPP_SOURCES} ${VCPP_HEADERS})
else()
    add_library(${PROJECT_NAME} STATIC ${VCPP_SOURCES} ${VCPP_HEADERS})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_VIDEO_CAPTURE)
target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${video_capture_VERSION} SOVERSION ${video_capture_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)

if (WIN32)
    # FFMPEG must be installed at the environment variable %FFMPEG_DIR%
    target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/lib")    
    target_link_libraries(${PROJECT_NAME} PRIVATE ${VCPP_FFMPEG_LIBS})
    target_include_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/include")
    foreach(DLL ${VCPP_FFMPEG_LIBS})
        file(COPY "$ENV{FFMPEG_DIR}/bin/" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILES_MATCHING PATTERN "${DLL}*.dll*")
    endforeach()
endif()

if(UNIX AND NOT APPLE)
    set(FFMPEG_LIBS "")
    foreach(LIB ${VCPP_FFMPEG_LIBS})
        list(APPEND FFMPEG_LIBS "lib${LIB}")
    endforeach()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ${FFMPEG_LIBS})
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FFMPEG)
endif()
