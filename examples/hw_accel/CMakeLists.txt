set(TARGET_NAME hw_accel)
add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

set(VCPP_FFMPEG_LIBS
    avutil 
    avformat
    avdevice 
    avcodec
    swscale
)

# Windows
if(WIN32)
    # FFMPEG must be installed at the environment variable %FFMPEG_DIR%
    target_link_directories(${TARGET_NAME} PRIVATE "$ENV{FFMPEG_DIR}/lib/")    
    target_link_libraries(${TARGET_NAME} PRIVATE ${VCPP_FFMPEG_LIBS})
    target_include_directories(${TARGET_NAME} PRIVATE "$ENV{FFMPEG_DIR}/include/")
    foreach(DLL ${VCPP_FFMPEG_LIBS})
        file(COPY "$ENV{FFMPEG_DIR}/bin/" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILES_MATCHING PATTERN "${DLL}*.dll*")
    endforeach()
endif()

# Linux
if(UNIX AND NOT APPLE)
    set(FFMPEG_LIBS "")
    foreach(LIB ${VCPP_FFMPEG_LIBS})
        list(APPEND FFMPEG_LIBS "lib${LIB}")
    endforeach()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ${FFMPEG_LIBS})
    target_link_libraries(${TARGET_NAME} PRIVATE PkgConfig::FFMPEG)
endif()