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
#enable_testing()
#add_subdirectory(test)

# The basic_usage demo
#if (MUDUO_BUILD_EXAMPLES)
    
#endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
