

find_program(LC_DOC_CLANG_EXECUTABLE clang NO_CACHE)

if (LC_DOC_CLANG_EXECUTABLE STREQUAL "LC_DOC_CLANG_EXECUTABLE-NOTFOUND")
	message(FATAL_ERROR "could not fund clang")
endif()

message("RESULT: ${LC_DOC_CLANG_EXECUTABLE}")
message("RESULT: ${LC_DOC_CLANG_EXECUTABLEFOUND}")

get_filename_component(LC_DOC_CLANG_BIN_DIR ${LC_DOC_CLANG_EXECUTABLE} DIRECTORY)
message("RESULT: ${LC_DOC_CLANG_BIN_DIR}")
get_filename_component(LC_DOC_CLANG_DIR ${LC_DOC_CLANG_BIN_DIR} DIRECTORY)
message("RESULT: ${LC_DOC_CLANG_DIR}")
set(LC_DOC_CLANG_LIB_DIR "${LC_DOC_CLANG_DIR}/lib")
message("RESULT: ${LC_DOC_CLANG_LIB_DIR}")
set(LC_DOC_CLANG_INCLUDE_DIR "${LC_DOC_CLANG_DIR}/include")
message("RESULT: ${LC_DOC_CLANG_LIB_DIR}")
set(LC_DOC_CLANG_LIBCLANG "${LC_DOC_CLANG_LIB_DIR}/libclang.lib")
message("RESULT: ${LC_DOC_CLANG_LIBCLANG}")