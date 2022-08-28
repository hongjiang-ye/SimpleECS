cmake_minimum_required(VERSION 3.5)

project (EcsDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/demo.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)
