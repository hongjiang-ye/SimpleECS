cmake_minimum_required(VERSION 3.14)

project(ecs_test)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} tests.cpp)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		ecs
		GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
