
cmake_minimum_required(VERSION 3.20)

project(
	LC_doc
	VERSION 0.0.0
	LANGUAGES CXX
	DESCRIPTION "Clang based documentation generator"
	HOMEPAGE_URL "https://github.com/LucaCiucci/LC_doc"
)

set(LC_DOC_MAIN_PROJECT FALSE)
set(LC_DOC_MAIN_PROJECT2 FALSE)
if ((CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR) AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
	set(LC_DOC_MAIN_PROJECT TRUE)
endif()

include(cmake/libclang.cmake)

include(FetchContent)
FetchContent_Declare(
  cmrc
  GIT_REPOSITORY https://github.com/vector-of-bool/cmrc
  GIT_TAG        2.0.1
)
FetchContent_GetProperties(cmrc)
if(NOT cmrc_POPULATED)
	FetchContent_Populate(cmrc)
	include("${cmrc_SOURCE_DIR}/CMakeRC.cmake")
endif()

# jbeder/yaml-cpp
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        yaml-cpp-0.7.0
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
	p-ranav-glob
	GIT_REPOSITORY https://github.com/LucaCiucci/glob
)
FetchContent_MakeAvailable(p-ranav-glob)

FetchContent_Declare(
	inja
	GIT_REPOSITORY https://github.com/pantor/inja
)
if(NOT inja_POPULATED)
	FetchContent_Populate(inja)
endif()

FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
	efsw
	#GIT_REPOSITORY https://github.com/SpartanJ/efsw
	GIT_REPOSITORY https://github.com/LucaCiucci/efsw
)
FetchContent_MakeAvailable(efsw)

add_subdirectory(apps)

install(SCRIPT "${CMAKE_SOURCE_DIR}/cmake/post_install.cmake")