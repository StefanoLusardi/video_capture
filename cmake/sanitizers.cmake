function(add_sanitizer TARGET)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Sanitizers enabled: address,undefined,memory,thread")
        target_compile_options(${TARGET} PRIVATE -g -fsanitize=address,undefined,memory,thread)
        target_compile_options(${TARGET} PRIVATE -fno-sanitize=signed-integer-overflow)
        target_compile_options(${TARGET} PRIVATE -fno-sanitize-recover=all)
        target_compile_options(${TARGET} PRIVATE -fno-omit-frame-pointer)
        target_link_libraries(${TARGET} PRIVATE -fsanitize=address,undefined,memory,thread -fuse-ld=gold)
    elseif(MSVC)
        message(STATUS "Sanitizers enabled: address")
        target_compile_options(${TARGET} PRIVATE -fsanitize=address)
    endif()
endfunction()