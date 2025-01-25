project(ecs-basic VERSION 0.1.0)

set(ECS_DIR ../../ecs/src)
include_directories(${ECS_DIR}/ )

add_executable(${PROJECT_NAME} basic.cpp)
target_link_libraries(${PROJECT_NAME})
