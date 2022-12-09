

# copy install_manifest.txt in the build directory to the install directory
# so that it can be used by the uninstall target

set(TMP_LIBMANAGER_DIR "${CMAKE_INSTALL_PREFIX}/lib_managers/LC_doc")
message(STATUS "TMP_LIBMANAGER_DIR: ${PROJECT_NAME}")

# if the file TMP_LIBMANAGER_DIR/install_manifest.txt exists, load it, add the missing
# lines in the current install_manifest.txt to it, and write it back out
if(EXISTS "${TMP_LIBMANAGER_DIR}/install_manifest.txt")

	# read ${TMP_LIBMANAGER_DIR}/install_manifest.txt into a list
	file(READ "${TMP_LIBMANAGER_DIR}/install_manifest.txt" TMP_MANIFEST)
	
	# split the list into lines
	# see https://cmake.org/pipermail/cmake/2007-May/014222.html
	STRING(REGEX REPLACE ";" "\\\\;" TMP_MANIFEST "${TMP_MANIFEST}")
	STRING(REGEX REPLACE "\n" ";" TMP_MANIFEST "${TMP_MANIFEST}")

	file(READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" MANIFEST)
	STRING(REGEX REPLACE ";" "\\\\;" MANIFEST "${MANIFEST}")
	STRING(REGEX REPLACE "\n" ";" MANIFEST "${MANIFEST}")
	
	# add the missing lines to the list
	foreach(line ${MANIFEST})
		list(FIND TMP_MANIFEST ${line} index)
		if(index EQUAL -1)
			list(APPEND TMP_MANIFEST ${line})
		endif()
	endforeach()

	STRING(REGEX REPLACE ";" "\n" TMP_MANIFEST "${TMP_MANIFEST}")
	
	# write the list back out to ${TMP_LIBMANAGER_DIR}/install_manifest.txt
	file(WRITE "${TMP_LIBMANAGER_DIR}/install_manifest.txt" "${TMP_MANIFEST}")
else()
	file(COPY "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" DESTINATION "${TMP_LIBMANAGER_DIR}")
endif()

message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/uninstall.cmake" DESTINATION "${TMP_LIBMANAGER_DIR}")