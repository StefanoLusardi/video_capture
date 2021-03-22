conan_cmake_run(CONANFILE conanfile.txt NO_OUTPUT_DIRS BASIC_SETUP KEEP_RPATHS BUILD missing)

add_executable(video_player src/main.cpp)
target_link_libraries(video_player 
    PRIVATE video_capture
    PRIVATE ${CONAN_LIBS}
)
