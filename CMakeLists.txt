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
        src/utils.c
        src/tsp.c
        ${HEADER_FILES}
        src/cplex_builder.c
        src/cplex_builder.h
        src/common.h
        src/tsp_common.c
        src/tsp_common.h
        src/cplex_callbacks.c
        src/cplex_callbacks.h
        src/cplex_common.c
        src/cplex_common.h
        src/common.c
)
target_include_directories(or2_project_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main src/main.c)
target_link_libraries(main or2_project_lib cplex m pthread dl)

add_executable(benchmark src/benchmark.c)
target_link_libraries(benchmark or2_project_lib cplex m pthread dl)

add_executable(batch_run_tsp batch_run_tsp.c)

# Custom target to run main on all .tsp files in data/ with cplex_callback_candidate
add_custom_target(run_all_tsp
    COMMAND bash -c 'for f in ${CMAKE_SOURCE_DIR}/data/tsp/*.tsp; do echo "Running on $f"; ${CMAKE_BINARY_DIR}/main -m cplex_callback_candidate --log ${CMAKE_BINARY_DIR}/logfile.log "$f"; done'
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running main on all .tsp files in data/ with cplex_callback_candidate method"
)

add_subdirectory(tests)
