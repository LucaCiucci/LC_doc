# run this script and define NAME
# cmake -P uninstall.cmake -DNAME=LC_doc

# load all the lines in install_manifest.txt
file(READ "${NAME}/install_manifest.txt" MANIFEST)
STRING(REGEX REPLACE ";" "\\\\;" contents "${MANIFEST}")
STRING(REGEX REPLACE "\n" ";" contents "${MANIFEST}")

# delete all the files in the list
foreach(file ${contents})
  message(STATUS "Removing ${file}")
  file(REMOVE "${file}")
endforeach()