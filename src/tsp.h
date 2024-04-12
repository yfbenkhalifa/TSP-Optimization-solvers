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

typedef struct{
    int covered_nodes_count;
    int* covered_nodes;
    int uncovered_nodes_count;
    int* uncovered_nodes;
}heuristic_state;


void tsp_greedy(instance* inst, int starting_node);
void tsp_extra_mileage(instance* inst, pair starting_pair);
solution euclidean_nearest_node(instance* instance, int node, int* remaining_nodes, int * remaining_nodes_count);
double compute_solution_cost(instance* inst, const int* solution);
pair euclidean_most_distant_pair(instance* inst);

void initialize_instance(instance* inst, heuristic_state* state);