project(test-archetype VERSION 0.1.0)

include_directories(
    ${ECS_DIR}/include
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME})
