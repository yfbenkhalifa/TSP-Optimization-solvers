//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "tsp.c"
#include "tsp_common.c"
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
    init_solution(&inst, solution);
    random_solution(&inst, solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution), true);
}



TEST(TspTests, GreedyHeuristicTest)
{
    instance inst = read_instance();
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    init_solution(&inst, solution->solution);
    tsp_grasp(&inst, solution, 0);
    memcpy(solution, inst.solution, inst.nnodes * sizeof(int));
    double final_cost = compute_solution_cost(&inst, solution->solution);
    EXPECT_EQ(sizeof(solution) / sizeof(solution[0]), inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
    log_results_to_csv("TSP_GRASP.csv", inst.input_file, "GRASP", final_cost, inst.elapsed_time);

}

TEST(TspTests, ExtraMileageTest)
{
    instance inst = read_instance();
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    init_solution(&inst, solution->solution);
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, solution, starting_pair);
    double final_cost = compute_solution_cost(&inst, solution->solution);
    EXPECT_GE(final_cost, 0);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
}

TEST(TspTests, TwoOptMoveTest) {
    instance inst = read_instance();
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    tsp_grasp(&inst, solution, 0);
    Edge e1, e2;
    e1.node1 = 0;
    e1.node2 = solution->solution[0];
    e2.node1 = 1;
    e2.node2 = solution->solution[1];
    two_opt_swap(solution->solution, inst.nnodes, e1, e2);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
}

TEST(TspTests, ExtraMileageWithTwoOptTest)
{
    instance inst = read_instance();
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    init_solution(&inst, solution->solution);
    pair starting_pair = euclidean_most_distant_pair(&inst);
    tsp_extra_mileage(&inst, solution, starting_pair);
    double final_cost = compute_solution_cost(&inst, solution->solution);
    EXPECT_EQ(has_duplicates(&inst, solution->solution), false);
    double delta = tsp_two_opt(&inst, solution);
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution->solution);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
    EXPECT_LE(delta, 0);
}

TEST(TspTests, GreedyGraspWithTwoOptTest)
{
    instance inst = read_instance();
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    tsp_grasp(&inst, solution, 0);
    double final_cost = compute_solution_cost(&inst, solution->solution);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
    double deltaCost = tsp_two_opt(&inst, solution);
    double final_cost_after_two_opt = compute_solution_cost(&inst, solution->solution);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
    EXPECT_EQ(final_cost_after_two_opt <= final_cost, true);
    EXPECT_LE(deltaCost, 0);
}

TEST(TspTests, GenerateNeighboursTest)
{
    instance inst = read_instance();
    int solution[inst.nnodes];
    random_solution(&inst, solution);
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
    Solution *solution = (Solution*)malloc(sizeof(Solution));
    solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    tsp_grasp(&inst, solution, 0);
    tabu_search(&inst, solution->solution, solution, inst.nnodes);
    EXPECT_EQ(is_tsp_solution(&inst, solution->solution), true);
    double final_cost = compute_solution_cost(&inst, solution->solution);
    EXPECT_GT(final_cost, 0); // Ensure the cost is positive
}

TEST(TspTests, VnsTest) {
    instance inst = read_instance();
    Solution *initial_solution = (Solution*)malloc(sizeof(Solution));
    Solution *final_solution = (Solution*)malloc(sizeof(Solution));
    initial_solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    final_solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    tsp_grasp(&inst, initial_solution, 0);
    double greedy_final_cost = compute_solution_cost(&inst, initial_solution->solution);
    tsp_vns(&inst, initial_solution->solution, final_solution, 5);
    EXPECT_EQ(is_tsp_solution(&inst, final_solution->solution), true);
    double final_cost = compute_solution_cost(&inst, final_solution->solution);
    EXPECT_GT(final_cost, 0); // Ensure the cost is positive
    EXPECT_LE(final_cost, greedy_final_cost); // Ensure the cost is less than or equal to the greedy solution
}

TEST(TspTests, SimulatedAnnealing)
{
    instance inst = read_instance();
    Solution *initial_solution = (Solution*)malloc(sizeof(Solution));
    Solution *final_solution = (Solution*)malloc(sizeof(Solution));
    initial_solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    final_solution->solution = (int*)malloc(inst.nnodes * sizeof(int));
    tsp_grasp(&inst, initial_solution, 0);
    double greedy_final_cost = compute_solution_cost(&inst, initial_solution->solution);
    tsp_simulated_annealing(&inst, final_solution, initial_solution);
    EXPECT_EQ(is_tsp_solution(&inst, final_solution->solution), true);
    double final_cost = compute_solution_cost(&inst, final_solution->solution);
    EXPECT_GT(final_cost, 0); // Ensure the cost is positive
    EXPECT_LE(final_cost, greedy_final_cost); // Ensure the cost is less than or equal to the greedy solution
}
