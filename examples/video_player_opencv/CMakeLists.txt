set(TARGET_NAME video_player_opencv)

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

if(${VCPP_BUILD_SANITIZERS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Sanitizers enabled: address,undefined")
        target_compile_options(${TARGET_NAME} PRIVATE -g -fsanitize=address,undefined)
        target_compile_options(${TARGET_NAME} PRIVATE -fno-sanitize=signed-integer-overflow)
        target_compile_options(${TARGET_NAME} PRIVATE -fno-sanitize-recover=all)
        target_compile_options(${TARGET_NAME} PRIVATE -fno-omit-frame-pointer)
        target_link_libraries(${TARGET_NAME} PRIVATE -fsanitize=address,undefined -fuse-ld=gold)
    elseif(MSVC)
        message(STATUS "Sanitizers enabled: address")
        target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=address)
    endif()
endif()