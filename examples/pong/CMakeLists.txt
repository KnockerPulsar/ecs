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

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} -lraylib)
