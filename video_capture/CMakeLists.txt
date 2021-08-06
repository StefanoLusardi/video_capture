message("")
message(":::::::::::::::::::::::::")
message("::VideoCapture:: Library")
message("")

set(PROJECT_NAME video_capture)

set(VCPP_FFMPEG_LIBS
    avutil 
    avformat
    avdevice 
    avcodec
    swscale)

set(VCPP_SOURCES 
    src/video_capture.cpp
    src/hw_acceleration.hpp
    src/logger.hpp)

set(VCPP_HEADERS 
    include/video_capture/api.hpp
    include/video_capture/raw_frame.hpp
    include/video_capture/frame_queue.hpp
    include/video_capture/video_capture.hpp)

if (WIN32 AND NOT ${VCPP_BUILD_SHARED})
    message(STATUS "Windows static lib is not supported.") 
    message(STATUS "FFMPEG downloaded using scripts/install_ffmpeg.bat provides shared libraries only.") 
    set(VCPP_BUILD_SHARED ON)
endif()

if(${VCPP_BUILD_SHARED})
    message(STATUS "Build shared library")
    # Unix relocatable library: it can be searched in executable directory or inside /<EXE_DIR>/../lib
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    add_library(${PROJECT_NAME} SHARED ${VCPP_SOURCES} ${VCPP_HEADERS})
else()
    message(STATUS "Build static library")
    add_library(${PROJECT_NAME} STATIC ${VCPP_SOURCES} ${VCPP_HEADERS})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_VIDEO_CAPTURE)
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${video_capture_VERSION} SOVERSION ${video_capture_VERSION_MAJOR})

if(${VCPP_INTERNAL_LOGGER})
    target_compile_definitions(${PROJECT_NAME} PRIVATE VIDEO_CAPTURE_LOG_ENABLED)
endif()

# Windows
if(WIN32)
    # FFMPEG must be installed at the environment variable %FFMPEG_DIR%
    target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/lib/")    
    target_link_libraries(${PROJECT_NAME} PRIVATE ${VCPP_FFMPEG_LIBS})
    target_include_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/include/")
    foreach(DLL ${VCPP_FFMPEG_LIBS})
        file(COPY "$ENV{FFMPEG_DIR}/bin/" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILES_MATCHING PATTERN "${DLL}*.dll*")
    endforeach()
endif()

# Linux
if(UNIX AND NOT APPLE)
    set(FFMPEG_LIBS "")
    foreach(LIB ${VCPP_FFMPEG_LIBS})
        list(APPEND FFMPEG_LIBS "lib${LIB}")
    endforeach()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ${FFMPEG_LIBS})
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FFMPEG)
endif()

# # Sanitizers
# if(${VCPP_ADDRESS_SANITIZER})
#     if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#         message(FATAL_ERROR "Sanitizer supported only for gcc/clang")
#     endif()
#     message(STATUS "Address sanitizer enabled")
#     target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
#     target_compile_options(${PROJECT_NAME} PRIVATE -fno-sanitize=signed-integer-overflow)
#     target_compile_options(${PROJECT_NAME} PRIVATE -fno-sanitize-recover=all)
#     target_compile_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer)
#     target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined -fuse-ld=gold)
# endif()