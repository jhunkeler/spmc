configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/spmbuild
	${CMAKE_CURRENT_BINARY_DIR}
	COPYONLY
)

if (APPLE)
	add_executable(spm_realpath
		realpath.c
	)
	install(
			PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/spm_realpath
			DESTINATION bin
	)
endif ()

install(
	PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/spmbuild
	DESTINATION bin
)
