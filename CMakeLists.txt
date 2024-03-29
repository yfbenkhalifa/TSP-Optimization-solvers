cmake_minimum_required(VERSION 3.27)
project(or2_project CXX C)
set(CMAKE_C_STANDARD 23)
include(CTest)

# Use the CPLEX_HOME environment variable as the path to the CPLEX directory
set(CPLEX_DIR $ENV{CPLEX_HOME})

# Print the value of the CPLEX_DIR variable
message("CPLEX_DIR is set to ${CPLEX_DIR}")

# Include the CPLEX headers
include_directories(${CPLEX_DIR}/include/ilcplex)

# Link against the CPLEX library
link_directories(${CPLEX_DIR}/lib/x86-64_linux/static_pic)

add_executable(or2_project src/main.c
        src/utils.c
        src/utils.h
        src/tsp.c
        src/tsp.h
)

# Link the executable with the CPLEX library
target_link_libraries(or2_project cplex m pthread dl)

add_subdirectory(tests)
