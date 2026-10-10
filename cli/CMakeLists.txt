file(GLOB_RECURSE CLI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(oxyir-cli ${CLI_SOURCES})
target_compile_options(oxyir-cli PRIVATE ${FLAGS})
target_link_libraries(oxyir-cli PRIVATE oxyir)