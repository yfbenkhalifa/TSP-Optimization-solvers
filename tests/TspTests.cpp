//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "tsp.c"
#define ROOTDIR "../../"

instance read_instance()
{
    std::string file = ROOTDIR"data/tsp_mock.tsp";
    // Read input from file
    instance inst;
    strcpy(inst.input_file, "NULL");
    strcpy(inst.input_file, file.c_str());
    read_input(&inst);
    return inst;
}

TEST(TspTests, RandomSolutionTest) {
    instance inst = read_instance();
    int solution[inst.nnodes];
    random_solution(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
}

TEST(TspTests, GreedyHeuristicTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    init_solution(&inst, solution);
    tsp_greedy(&inst, 0);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);

}

TEST(TspTests, ExtraMileageTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    init_solution(&inst, solution);
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, starting_pair);
    for (int i = 0; i < inst.nnodes; i++)
    {
        solution[i] = inst.solution[i];
    }
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
}

TEST(TspTests, TwoOptMoveTest) {
    instance inst = read_instance();
    int solution[inst.nnodes];
    random_solution(&inst, solution);
    Edge e1, e2;
    e1.node1 = 0;
    e1.node2 = solution[0];
    e2.node1 = 1;
    e2.node2 = solution[1];
    two_opt_swap(solution, inst.nnodes, e1, e2);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
}

TEST(TspTests, ExtraMileageWithTwoOptTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    init_solution(&inst, solution);
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, starting_pair);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(has_duplicates(&inst, solution), false);
    double delta = tsp_two_opt(&inst);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
    EXPECT_LE(delta, 0);
}

TEST(TspTests, GreedyGraspWithTwoOptTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    tsp_greedy(&inst, 0);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
    double deltaCost = tsp_two_opt(&inst);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
    EXPECT_LE(deltaCost, 0);
}

TEST(TspTests, GenerateNeighboursTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    random_solution(&inst, solution);

    int num_neighbors = 2;
    int **neighbors = (int **)malloc(2 * sizeof(int *));
    generate_neighbors(solution, inst.nnodes, neighbors, num_neighbors);
    for (int i = 0; i < num_neighbors; i++)
    {
        EXPECT_EQ(is_2opt_neighbour(solution, neighbors[i], inst.nnodes), true);
    }
}

TEST(TspTests, Is2OptNeighbour) {
    instance inst = read_instance();
    int solution[inst.nnodes];
    random_solution(&inst, solution);
    int neighbor[inst.nnodes];
    two_opt_swap(neighbor, inst.nnodes, (Edge){0, solution[0]}, (Edge){2, solution[2]});
    EXPECT_EQ(is_2opt_neighbour(solution, neighbor, inst.nnodes), true);
    two_opt_swap(neighbor, inst.nnodes, (Edge){0, solution[0]}, (Edge){2, solution[2]});
    EXPECT_EQ(is_2opt_neighbour(solution, neighbor, inst.nnodes), false);
}

TEST(TspTests, TabuSearchTest) {
    instance inst = read_instance();
    int solution[inst.nnodes];

    // Initialize the solution with a starting point, e.g., a greedy solution
    tsp_greedy(&inst, 0);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));

    // Run Tabu Search
    tabu_search(solution, inst.nnodes);

    // Update the instance with the new solution
    memcpy(inst.solution, solution, inst.nnodes * sizeof(int));

    // Assertions
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
    double final_cost = compute_solution_cost(&inst, solution);
    EXPECT_GT(final_cost, 0); // Ensure the cost is positive
}
