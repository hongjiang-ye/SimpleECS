project(ecs_examples)

### basic demo
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage ecs)

### particle demo
add_executable(particle particle.cpp)

# glad
add_library(glad 
    ${PROJECT_SOURCE_DIR}/third-party/include/glad/glad.h 
    ${PROJECT_SOURCE_DIR}/third-party/src/glad.c
)
target_include_directories(glad 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/third-party/include
)

# GLFW lib
target_link_directories(particle
    PUBLIC
        ${PROJECT_SOURCE_DIR}/third-party/lib/
)

target_link_libraries(particle 
    ecs
    glad
    glfw3
    opengl32
)
target_include_directories(particle
    PUBLIC
        ${PROJECT_SOURCE_DIR}/third-party/include/
)
