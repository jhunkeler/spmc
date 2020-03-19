include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
)

add_executable(spm
	spm.c
)

target_link_libraries(spm libspm crypto ssl curl)
if (LINUX)
	target_link_libraries(spm libspm rt)
endif()

if(MSVC)
	target_compile_options(spm PRIVATE /W4 /WX)
else()
	target_compile_options(spm PRIVATE -Wall -Wextra -fstack-protector)
endif()

install(
	TARGETS spm
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
