cmake_minimum_required (VERSION 3.16)

project (video_capture 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "Video Capture wrapper for FFMPEG with Software/Hardware decoding, written in modern C++"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/video_capture"
)

message("System Info: " ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(ENV{CONAN_SYSREQUIRES_MODE} "enabled")
set(CONAN_SYSTEM_INCLUDES ON)
set(CONAN_CMAKE_SILENT_OUTPUT ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

add_subdirectory(video_capture)

option(VC_BUILD_TESTS "Build library tests" OFF)
option(VC_BUILD_EXAMPLES "Build library examples" ON)
option(USE_SYSTEM_FFMPEG "Use system FFMPEG" OFF)

if(${VC_BUILD_TESTS})
    add_subdirectory(tests)
endif()

if(${VC_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()
