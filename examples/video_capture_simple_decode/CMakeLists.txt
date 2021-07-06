set(TARGET_NAME video_capture_simple_decode)
add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE video_capture)

if(${VCPP_BUILD_SANITIZERS})
    include(sanitizers)
endif()