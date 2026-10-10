cmake_minimum_required(VERSION 4.1)
project(OxyIR VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(FLAGS -Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-local-typedefs -Wno-unused-lambda-capture -freflection)

add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(cli)
add_subdirectory(samples)