set(TARGET_NAME video_player_qt)

# Conan Qt6 Install
# conan_cmake_configure(
#     GENERATORS cmake_find_package
#     REQUIRES qt/6.0.2
#     OPTIONS qt:shared=True
#     IMPORTS 
#         "bin, Qt6Core*.dll -> ../../bin"
#         "bin, Qt6Gui*.dll -> ../../bin"
#         "bin, Qt6Widgets*.dll -> ../../bin"
# )
# conan_cmake_autodetect(conan_settings)
# conan_cmake_install(
#     PATH_OR_REFERENCE .
#     BUILD missing
#     SETTINGS ${conan_settings}
# )
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})
# list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/examples/${TARGET_NAME})

if(EXISTS "$ENV{QTDIR}")
    file(TO_CMAKE_PATH "$ENV{QTDIR}" QTDIR)
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED QUIET)

add_executable(${TARGET_NAME} WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/main_window.ui
    src/main_window.cpp
    src/video_widget.cpp
    include/main_window.hpp
    include/video_widget.hpp
    res/resources.qrc
)

target_include_directories(${TARGET_NAME} PRIVATE include/)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE Qt6::Widgets
)

if(WIN32)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        file(COPY "${QTDIR}/plugins/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/" FILES_MATCHING PATTERN "*d.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Cored.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Guid.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Widgetsd.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Svgd.dll")
    else()
        file(COPY "${QTDIR}/plugins/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/" PATTERN "*d.dll" EXCLUDE)
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Core.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Gui.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Widgets.dll")
        file(COPY "${QTDIR}/bin/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" FILES_MATCHING PATTERN "Qt6Svg.dll")
    endif()
    file(REMOVE_RECURSE  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/")
endif()

# Conan Qt6 Deploy
# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/qt.conf" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
# file(COPY "${Qt6_RES_DIRS}/archdatadir/plugins/" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/")
