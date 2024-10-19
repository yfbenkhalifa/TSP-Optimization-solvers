//
// Created by wiz on 3/29/24.
//

#ifndef OR2_PROJECT_TSP_H
#define OR2_PROJECT_TSP_H
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cplex.h>
#include <pthread.h>
#include "utils.h"
#endif //OR2_PROJECT_TSP_H

typedef struct
{
    int covered_nodes_count;
    int* covered_nodes;
    int uncovered_nodes_count;
    int* uncovered_nodes;
} heuristic_state;

typedef struct {
    int node1;
    int node2;
}Edge;

typedef struct {
    int *solution;
    int cost;
} Solution;

#define TABU_TENURE 5
#define MAX_ITERATIONS 100

bool is_tsp_solution(instance* inst, int* solution);
void init_solution(instance* inst, int* solution);
bool is_neighbor(int *solution, int *neighbor, int size);
void tsp_greedy(instance* inst, int starting_node);
void tsp_extra_mileage(instance* inst, pair starting_pair);
solution euclidean_nearest_node(instance* instance, int node, int* remaining_nodes, int* remaining_nodes_count);
double compute_solution_cost(instance* inst, const int* solution);
pair euclidean_most_distant_pair(instance* inst);
double tsp_two_opt(instance* inst);
void two_opt_swap(int *solution, int size, Edge e1, Edge e2);
void initialize_instance(instance* inst, heuristic_state* state);
int evaluate_solution(int *solution, int size);
void generate_neighbors(int *solution, int size, int **neighbours, int num_neighbors);
bool is_tabu(int *solution, int **tabu_list, int tabu_size, int size);
void add_to_tabu_list(int *solution, int **tabu_list, int *tabu_index, int size);
void random_solution(instance *inst, int *solution);
bool is_2opt_neighbour(int *solution1, int *solution2, int size);