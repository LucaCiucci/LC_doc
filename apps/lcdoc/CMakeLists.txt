


add_executable(lcdoc main.cpp "clang_interface/Cursor.cpp" "clang_interface/Index.cpp" "clang_interface/TranslationUnit.cpp" "html_page.cpp" "Symbol.cpp" "string_utils.cpp" "cxx_parser.cpp" "list_page.cpp" "Project.cpp")

target_link_libraries(lcdoc PRIVATE ${LC_DOC_CLANG_LIBCLANG})
target_include_directories(lcdoc PRIVATE ${LC_DOC_CLANG_INCLUDE_DIR})
set_target_properties(lcdoc PROPERTIES CXX_STANDARD 20)

cmrc_add_resource_library(
	lcdoc-resources
	ALIAS lcdoc::rc
	NAMESPACE lcdoc
	"main.cpp"
	"tooltips/int-keyword.html"
)
target_link_libraries(lcdoc PRIVATE lcdoc::rc)
target_link_libraries(lcdoc PRIVATE yaml-cpp)
target_link_libraries(lcdoc PRIVATE Glob)
target_link_libraries(lcdoc PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(lcdoc PRIVATE "${inja_SOURCE_DIR}/include")

target_compile_definitions(lcdoc PRIVATE LCDOC_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
