cmake_minimum_required(VERSION 3.6.0)   # Selects the minimum version of CMake required to run this file
project(ecs VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

CMAKE_POLICY(SET CMP0074 NEW)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -flto -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall")
set(ECS_DIR ecs/src CACHE PATH "Directory containing ECS header files")

add_subdirectory(examples/basic)
add_subdirectory(examples/pong)
add_subdirectory(tests/archetype)
