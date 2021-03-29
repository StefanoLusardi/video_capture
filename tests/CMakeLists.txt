message("")
message("::::::::::::::::::::::")
message("::VideoCapture:: Tests")
message("")

#include(CTest)
enable_testing()

set(TARGET_NAME video_capture_tests)

conan_cmake_configure(
    GENERATORS cmake_find_package
    REQUIRES gtest/1.10.0
    OPTIONS gtest:no_main=True
    OUTPUT_QUIET
)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${conan_settings}
    OUTPUT_QUIET
)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/tests)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/tests)

find_package(GTest COMPONENTS gtest REQUIRED QUIET)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/vc_test.cpp
	src/vc_fixture.hpp
)

target_link_libraries(${TARGET_NAME} 
	PRIVATE video_capture 
	PRIVATE GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
