include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
)

add_executable(spm spm.c config.c compat.c resolve.c fs.c rpath.c find.c shell.c archive.c str.c relocation.c install.c config_global.c manifest.c checksum.c extern/url.c version_spec.c spm_build.c mime.c internal_cmd.c environment.c mirrors.c strlist.c shlib.c)

target_link_libraries(spm crypto ssl curl)
if (LINUX)
	target_link_libraries(spm rt)
endif()

if(MSVC)
	target_compile_options(spm PRIVATE /W4 /WX)
else()
	target_compile_options(spm PRIVATE -Wall -Wextra)
endif()

install(
	TARGETS spm
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
