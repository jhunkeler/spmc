cmake_minimum_required(VERSION 3.11)
project(spm C)
include(CheckSymbolExists)
set(CMAKE_C_STANDARD 99)

check_symbol_exists(strsep string.h HAVE_STRSEP)
check_symbol_exists(reallocarray stdlib.h HAVE_REALLOCARRAY)
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config.h)

add_subdirectory(src)
