set(TARGET_NAME video_player_qt)

conan_cmake_configure(
    GENERATORS cmake_find_package
    REQUIRES qt/6.0.2
    OPTIONS qt:shared=True
    IMPORTS 
        "bin, Qt6Core*.dll -> ../../bin"
        "bin, Qt6Gui*.dll -> ../../bin"
        "bin, Qt6Widgets*.dll -> ../../bin"
)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${conan_settings}
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON) # TODO: Remove

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
message("Qt VERSION: " ${Qt6_VERSION})

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)
target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE Qt6::Widgets
)
set_target_properties(${TARGET_NAME} 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/qt.conf" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(WIN32)
    file(COPY "${Qt6_RES_DIRS}/archdatadir/plugins/platforms/" 
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platforms
        FILES_MATCHING PATTERN "qwindows*.dll")
endif()