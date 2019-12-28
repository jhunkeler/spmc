include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
)

add_executable(spm spm.c config.c compat.c deps.c fs.c rpath.c find.c shell.c archive.c strings.c relocation.c install.c config_global.c manifest.c checksum.c extern/url.c version_spec.c spm_build.c mime.c internal_cmd.c)
target_link_libraries(spm rt crypto ssl curl)
install(
	TARGETS spm
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
install(
	FILES
		${CMAKE_BINARY_DIR}/include/config.h
		${CMAKE_SOURCE_DIR}/include/spm.h
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}"
)