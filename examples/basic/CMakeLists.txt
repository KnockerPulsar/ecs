cmake_minimum_required(VERSION 3.0.0)
project(ecs-basic VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(ECS_DIR ../../ecs/)

include_directories( 
	${ECS_DIR}/include
)

add_executable(${PROJECT_NAME} basic.cpp)
target_link_libraries(${PROJECT_NAME})
