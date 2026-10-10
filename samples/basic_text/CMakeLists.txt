add_executable(basic_text main.cpp)
target_compile_options(basic_text PRIVATE ${FLAGS})
target_link_libraries(basic_text PRIVATE oxyir)