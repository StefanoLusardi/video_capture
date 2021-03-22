cmake_minimum_required (VERSION 3.16)

project (video_player 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "VideoPlayer"
    HOMEPAGE_URL "https://github.com/StefanoLusardi"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(run_conan)

message("CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message("CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME})
message("CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION})
message("CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})

add_subdirectory(video_capture)
add_subdirectory(video_player)

