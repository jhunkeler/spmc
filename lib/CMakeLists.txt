include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
)

set(libspm_src
	config.c
	compat.c
	resolve.c
	fs.c
	rpath.c
	find.c
	shell.c
	archive.c
	str.c
	relocation.c
	install.c
	config_global.c
	manifest.c
	checksum.c
	extern/url.c
	version_spec.c
	spm_build.c
	mime.c
	internal_cmd.c
	environment.c
	mirrors.c
	strlist.c
	shlib.c
	user_input.c
	metadata.c
	purge.c
)

add_library(libspm_obj OBJECT ${libspm_src})
set_property(TARGET libspm_obj PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(libspm SHARED $<TARGET_OBJECTS:libspm_obj>)
add_library(libspm_static STATIC $<TARGET_OBJECTS:libspm_obj>)


target_link_libraries(libspm crypto ssl curl)
if (LINUX)
	target_link_libraries(libspm rt)
endif()

if(MSVC)
	target_compile_options(libspm PUBLIC /W4 /WX)
else()
	target_compile_options(libspm PUBLIC -Wall -Wextra -fstack-protector)
endif()

set_target_properties(libspm PROPERTIES OUTPUT_NAME "spm")
install(
	TARGETS libspm
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
