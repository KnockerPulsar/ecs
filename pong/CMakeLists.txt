cmake_minimum_required(VERSION 3.0.0)   # Selects the minimum version of CMake required to run this file
project(ecs-pong VERSION 0.1.0)          # Here we select the project name and version

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -flto -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall")

CMAKE_POLICY(SET CMP0074 NEW)

set(ECS_DIR ../ecs/src)
set(RAYLIB_DIR ./deps/raylib)

set(SOURCES 
)

# Define the directories in which to search for the included headers
include_directories(
  src/
  ${ECS_DIR}/
  ${RAYLIB_DIR}/include
)

link_directories(${RAYLIB_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG    ${PROJECT_SOURCE_DIR}/build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${PROJECT_SOURCE_DIR}/build/release)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lraylib)
