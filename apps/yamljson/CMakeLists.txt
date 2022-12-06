

add_executable(yamljson main.cpp)

target_link_libraries(yamljson PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(yamljson PRIVATE yaml-cpp)

install(TARGETS yamljson RUNTIME DESTINATION bin)

set_property(TARGET yamljson PROPERTY CXX_STANDARD 20)