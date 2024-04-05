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

void tsp_greedy(instance* inst, int* solution, int starting_node, bool round, bool verbose, bool randomize);
int nearest_node(instance* instance, int node, int* remaining_nodes, int * remaining_nodes_count, bool round);