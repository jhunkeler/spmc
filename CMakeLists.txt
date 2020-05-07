cmake_minimum_required(VERSION 3.11)
project(spm C)

include(FindPkgConfig)
include(CTest)
include(CheckSymbolExists)

set(CMAKE_C_STANDARD 99)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)

enable_testing()
check_symbol_exists(strsep string.h HAVE_STRSEP)
check_symbol_exists(reallocarray stdlib.h HAVE_REALLOCARRAY)
pkg_check_modules(OpenSSL openssl>=1.1)
pkg_check_modules(CURL libcurl>=7.0)
find_program(RELOC reloc)
find_program(TAR tar)
find_program(WHICH which)
find_program(FILE file)
find_program(OBJDUMP objdump)
find_program(RSYNC rsync)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config.h)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(scripts)
add_subdirectory(tests)
