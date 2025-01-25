project(ecs-pong VERSION 0.1.0)          # Here we select the project name and version

CMAKE_POLICY(SET CMP0074 NEW)

set(ECS_DIR ../../ecs/src)
set(RAYLIB_DIR ./deps/raylib)

set(SOURCES )

# Define the directories in which to search for the included headers
include_directories(
  ${ECS_DIR}/
  ${RAYLIB_DIR}/include
  src/
  src/levels/
)

link_directories(${RAYLIB_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG    ${PROJECT_SOURCE_DIR}/build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${PROJECT_SOURCE_DIR}/build/release)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lraylib)
