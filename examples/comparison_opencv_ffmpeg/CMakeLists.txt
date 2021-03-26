set(TARGET_NAME comparison_opencv_ffmpeg)

set(TARGET_NAME comparison_opencv_ffmpeg)

conan_cmake_configure(
    REQUIRES opencv/4.5.1
    GENERATORS cmake_find_package
)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${conan_settings}
)
    
find_package(OpenCV COMPONENTS core highgui REQUIRED)
message("OpenCV_VERSION: " ${OpenCV_VERSION})
    
add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)
target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE opencv::core
    PRIVATE opencv::highgui
)

set_target_properties(${TARGET_NAME} 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)