cmake_minimum_required(VERSION 3.15)
project(spm C)
include(CheckSymbolExists)
find_package(Git)
set(CMAKE_C_STANDARD 99)
execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --always --tags --dirty
        OUTPUT_VARIABLE VERSION
        ERROR_VARIABLE VERSION_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(${VERSION_ERROR})
    message("Git required to build this software")
    return(1)
endif(${VERSION_ERROR})
check_symbol_exists(strsep string.h HAVE_STRSEP)
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config.h)

add_subdirectory(src)
