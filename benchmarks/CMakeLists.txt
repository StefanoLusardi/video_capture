message("")
message("::::::::::::::::::::::")
message("::VideoCapture:: Benchmarks ")
message("")

set(TARGET_NAME benchmark_opencv)

conan_cmake_configure(
    REQUIRES cppbenchmark/cci.20201029
    REQUIRES opencv/4.5.1
    GENERATORS cmake_find_package
    OUTPUT_QUIET
)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${conan_settings}
    OUTPUT_QUIET
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/benchmarks)

find_package(cppbenchmark REQUIRED QUIET)
find_package(OpenCV COMPONENTS core videoio REQUIRED QUIET)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE opencv::core
    PRIVATE opencv::videoio
    PRIVATE cppbenchmark::cppbenchmark
)
