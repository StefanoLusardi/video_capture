set(TARGET_NAME video_player_imgui)

conan_cmake_configure(
    REQUIRES imgui/1.87
    REQUIRES glfw/3.3.7
    GENERATORS cmake_find_package
    IMPORTS "./res/bindings, imgui_impl_glfw.h -> ../bindings"
    IMPORTS "./res/bindings, imgui_impl_glfw.cpp -> ../bindings"
    IMPORTS "./res/bindings, imgui_impl_opengl3.h -> ../bindings"
    IMPORTS "./res/bindings, imgui_impl_opengl3.cpp -> ../bindings"
    IMPORTS "./res/bindings, imgui_impl_opengl3_loader.h  -> ../bindings"
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

find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp
    ${CMAKE_BINARY_DIR}/examples/bindings/imgui_impl_glfw.cpp
	${CMAKE_BINARY_DIR}/examples/bindings/imgui_impl_glfw.h
	${CMAKE_BINARY_DIR}/examples/bindings/imgui_impl_opengl3.cpp
	${CMAKE_BINARY_DIR}/examples/bindings/imgui_impl_opengl3.h
)

target_include_directories(${TARGET_NAME} 
	PRIVATE ${CMAKE_BINARY_DIR}/examples/bindings
)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_capture 
    PRIVATE imgui::imgui
    PRIVATE glfw
)

if(${VCPP_BUILD_SANITIZERS})
    include(sanitizers)
endif()