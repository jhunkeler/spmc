install(
	FILES
		${CMAKE_BINARY_DIR}/include/config.h
		archive.h
		checksum.h
		conf.h
		deps.h
		environment.h
		find.h
		fs.h
		install.h
		internal_cmd.h
		manifest.h
		mime.h
		mirrors.h
		relocation.h
		rpath.h
		shell.h
		shlib.h
		spm.h
		str.h
		strlist.h
		url.h
		version_spec.h

	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}"
)
