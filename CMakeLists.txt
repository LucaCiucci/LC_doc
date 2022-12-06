
cmake_minimum_required(VERSION 3.20)

project(
	LC_doc
	VERSION 0.0.0
	LANGUAGES CXX
	DESCRIPTION "Clang based documentation generator"
	HOMEPAGE_URL "https://github.com/LucaCiucci/LC_doc"
)

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
	GIT_REPOSITORY https://github.com/p-ranav/glob
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

add_subdirectory(apps)