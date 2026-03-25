project(ecs-basic VERSION 0.1.0)

message("ECS_DIR IS ${ECS_DIR}")
include_directories(
    ${ECS_DIR}/include
)

add_executable(${PROJECT_NAME} basic.cpp)
target_link_libraries(${PROJECT_NAME})
