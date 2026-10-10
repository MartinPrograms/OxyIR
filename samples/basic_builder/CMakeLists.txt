add_executable(basic_builder main.cpp)
target_compile_options(basic_builder PRIVATE ${FLAGS})
target_link_libraries(basic_builder PRIVATE oxyir)