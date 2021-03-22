set(PROJECT_NAME video_capture_tests)

include(DownloadGoogleTest)
include(GoogleTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/vc_test.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE video_capture gtest)

gtest_discover_tests(${PROJECT_NAME})
