cmake_minimum_required(VERSION 3.14)

project (ecs_demo)

set(CMAKE_CXX_STANDARD 17)

# The header-only ECS library
add_library(ecs INTERFACE)
target_include_directories(ecs
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include/
)

# The gtest
enable_testing()
add_subdirectory(test)

# The demo
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/demo.cpp)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ecs
)
