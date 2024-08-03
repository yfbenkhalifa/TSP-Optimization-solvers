//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "tsp.c"
#define ROOTDIR "../../"

instance read_instance()
{
    std::string file = ROOTDIR"data/att48.tsp";
    // Read input from file
    instance inst;
    strcpy(inst.input_file, "NULL");
    strcpy(inst.input_file, file.c_str());
    read_input(&inst);
    return inst;
}

TEST(TspTests, NearestNodeTest)
{

}


TEST(TspTests, GreedyHeuristicTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    tsp_greedy(&inst, 0);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);

}

TEST(TspTests, ExtraMileageTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, starting_pair);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
}

TEST(TspTests, ExtraMileageWithTwoOptTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, starting_pair);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
    tsp_two_opt(&inst);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
}

TEST(TspTests, GreedyGraspWithTwoOptTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_greedy(&inst, 0);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
    double deltaCost = -1;

    deltaCost = tsp_two_opt(&inst);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
}
