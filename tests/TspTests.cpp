//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "tsp.c"
#define ROOTDIR "../../"

instance read_instance(){
    std::string file = ROOTDIR"data/att48.tsp";
    // Read input from file
    instance inst;
    strcpy(inst.input_file, "NULL");
    strcpy(inst.input_file, file.c_str());
    read_input(&inst);
    return inst;
}

TEST(TspTests, NearestNodeTest) {
}

TEST(TspTests,  GreedyHeuristicTest){
    instance inst = read_instance();
    int solution [inst.nnodes];
    tsp_greedy(&inst, solution, 0, true, false, true);
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
}