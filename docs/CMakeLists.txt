find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/icon.png")
    set(VCPP_HEADERS_PATH "${CMAKE_SOURCE_DIR}/video_capture/include/video_capture")
    set(VCPP_DOCS_HEADERS 
        ${VCPP_HEADERS_PATH}/frame_queue.hpp
        ${VCPP_HEADERS_PATH}/raw_frame.hpp
        ${VCPP_HEADERS_PATH}/video_capture.hpp
    )

    set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/video_capture/include/video_capture)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/)
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
        DEPENDS ${VCPP_DOCS_HEADERS}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating documentation using Doxygen"
    )

    add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
else()
    message(WARNING "Please install Doxygen to generate documentation.")
endif()