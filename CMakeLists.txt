cmake_minimum_required(VERSION 3.15)
project(spm C)
include(CheckSymbolExists)

set(CMAKE_C_STANDARD 99)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
check_symbol_exists(strsep string.h HAVE_STRSEP)
configure_file(config.h.in config.h)

add_executable(spm spm.c config.c spm.h config.h.in compat.c)
target_link_libraries(spm rt)
