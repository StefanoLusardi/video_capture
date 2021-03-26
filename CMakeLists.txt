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
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CONAN_SYSTEM_INCLUDES ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

message("CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message("CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME})
message("CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION})
message("CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})

add_subdirectory(video_capture)
# add_subdirectory(video_player)

option(VC_BUILD_TESTS "Build library tests" OFF)
option(VC_BUILD_EXAMPLES "Build library examples" ON)

if(${VC_BUILD_TESTS})
	message(STATUS "::VideoCapture:: Building tests")
	include(CTest)
	enable_testing()
	add_subdirectory(tests)
endif()

if(${VC_BUILD_EXAMPLES})
	message(STATUS "::VideoCapture:: Build examples")
	add_subdirectory(examples)
endif()
