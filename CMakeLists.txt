cmake_minimum_required(VERSION 3.6)
project(or2_project CXX C)

set(CPLEX_DIR $ENV{CPLEX_HOME})
set(HEADER_FILES
        src/utils.h
        src/tsp.h
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

include(CTest)

# Print the value of the CPLEX_DIR variable
message("CPLEX_DIR is set to ${CPLEX_DIR}")

# Include the CPLEX headers
include_directories(${CPLEX_DIR}/include/ilcplex)

# Link against the CPLEX library
link_directories(${CPLEX_DIR}/lib/x86-64_linux/static_pic)

add_library(or2_project_lib
        src/utils.c
        src/tsp.c
        ${HEADER_FILES}
        src/cplex_builder.c
        src/cplex_builder.h
)
target_include_directories(or2_project_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(or2_project src/main.c
        src/utils.c
        src/tsp.c
        ${HEADER_FILES}
        src/cplex_builder.c
        src/cplex_builder.h
        src/common.h
        src/tsp_common.c
        src/tsp_common.h

)


# Link the executable with the CPLEX library
target_link_libraries(or2_project cplex m pthread dl)
add_subdirectory(tests)
