include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/include
	${OpenSSL_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
)

add_executable(spm
	spm.c
)

target_link_directories(spm PUBLIC ${OpenSSL_LIBRARY_DIRS} ${CURL_LIBRARY_DIRS})
target_link_libraries(spm libspm ${OpenSSL_LIBRARIES} ${CURL_LIBRARIES})

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
