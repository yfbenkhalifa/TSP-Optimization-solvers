cmake_minimum_required(VERSION 3.14)
project(or2_project_tests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(PARENT_DIR ../ ABSOLUTE)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
include_directories(${PARENT_DIR}/src)

add_executable(unit_tests
        UnitTests.cpp
        TspTests.cpp
        CplexTests.cpp
)
target_link_libraries(
        unit_tests
        GTest::gtest_main
        or2_project_lib
        cplex
        dl
)

include(GoogleTest)
gtest_discover_tests(unit_tests)