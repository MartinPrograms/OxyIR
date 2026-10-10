file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(oxyir ${SOURCES})
target_compile_options(oxyir PRIVATE ${FLAGS})
target_include_directories(oxyir PUBLIC ${CMAKE_SOURCE_DIR}/builder ${CMAKE_SOURCE_DIR}/src)