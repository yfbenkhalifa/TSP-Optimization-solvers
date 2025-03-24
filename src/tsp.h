//
// Created by wiz on 3/29/24.
//

#ifndef OR2_PROJECT_TSP_H
#define OR2_PROJECT_TSP_H
#include <stdbool.h>
#include "tsp_common.h"
#include "common.h"
#include "utils.h"



#define TABU_TENURE 5
#define MAX_ITERATIONS 100


void init_solution(instance* inst, int* solution);
void generate_random_solution(instance *inst, int *solution);
bool is_neighbor(const int *solution, const int *neighbor, int size);
void tsp_grasp(instance* inst, Solution *solution, int starting_node);
void tsp_extra_mileage(instance* inst, Solution *solution, pair starting_pair);
void tabu_search(instance* inst, int *initial_solution, Solution *solution, int size);
void tsp_vns(instance* inst, int *initial_solution, int size);
solution_node euclidean_nearest_node(instance* instance, int node, int* remaining_nodes, int* remaining_nodes_count);
pair euclidean_most_distant_pair(instance* inst);
double tsp_two_opt(instance* inst, Solution *solution);
void two_opt_swap(int *solution, int size, Edge e1, Edge e2);
void initialize_instance(instance* inst, heuristic_state* state);
int evaluate_solution(int *solution, int size);
void generate_neighbors(int *solution, int size, int **neighbours, int num_neighbors);
bool is_tabu(int *solution, int **tabu_list, int tabu_size, int size);
void add_to_tabu_list(int *solution, int **tabu_list, int *tabu_index, int size);
void random_solution(instance *inst, int *solution);
bool is_2opt_neighbour(int *solution1, int *solution2, int size);
void tsp_simulated_annealing(instance *instance, Solution *solution, Solution *initial_solution);

#endif //OR2_PROJECT_TSP_H