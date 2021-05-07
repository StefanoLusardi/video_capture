set(TARGET_NAME video_player_queue)

conan_cmake_configure(
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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})

find_package(OpenCV COMPONENTS core highgui REQUIRED QUIET)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE opencv::core
    PRIVATE opencv::highgui
)