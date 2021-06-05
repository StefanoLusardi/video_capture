message("")
message(":::::::::::::::::::::::::")
message("::VideoCapture:: Library")
message("")

set(PROJECT_NAME video_capture)
add_library(${PROJECT_NAME} SHARED src/video_capture.cpp src/frame_sync.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_VIDEO_CAPTURE)
target_include_directories(${PROJECT_NAME}
    PRIVATE include/${PROJECT_NAME}
    PUBLIC include)

set(FFMPEG_LIBS avutil avformat avdevice avcodec swscale)

if (WIN32)
    # FFMPEG must be installed at the environment variable %FFMPEG_DIR%
    target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/lib")    
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBS})
    target_include_directories(${PROJECT_NAME} PRIVATE "$ENV{FFMPEG_DIR}/include")

    foreach(DLL ${FFMPEG_LIBS})
        message("Copying ${DLL} from $ENV{FFMPEG_DIR}/bin to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        file(COPY "$ENV{FFMPEG_DIR}/bin/"
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            FILES_MATCHING PATTERN "${DLL}*.dll*")
    endforeach()
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ${FFMPEG_LIBS})
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FFMPEG)
endif()



#     if(UNIX)
#         message(STATUS "Create symlinks for FFMPEG libs")
#         set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/${CMAKE_SYSTEM_NAME}/lib")
#         FILE(GLOB LIBS "${FFMPEG_LIB_DIR}/lib*.so.*")
#         foreach(LIB ${LIBS})
#             string(REGEX MATCH ".*.so" LIB_SYMLINK ${LIB})
#             message("lib:     ${LIB}")
#             message("symlink: ${LIB_SYMLINK}")
#             execute_process(COMMAND ln -sf "${LIB}" "${LIB_SYMLINK}")
#         endforeach()

#         message(STATUS "Copy FFMPEG binaries")
#         file(COPY "${FFMPEG_LIB_DIR}/"
#             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#             FILES_MATCHING PATTERN "*.so*")
#     endif()
    
# endif()