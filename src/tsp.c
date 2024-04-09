//
// Created by wiz on 3/29/24.
//

#include "tsp.h"
#define VERBOSE 50


void tsp_greedy(instance* inst, int* solution, int starting_node, bool round, bool verbose, bool randomize){
    int current_node_index = 0;
    int remaining_nodes_count = inst->nnodes;
    int remaining_nodes [inst->nnodes];
    int nearest_node_index = -1;

    // Initialize solution
    for(int i = 0; i < inst->nnodes; i++){
        solution[i] = -1;
        remaining_nodes[i] = i;
    }

    if (randomize){
        current_node_index = rand() % inst->nnodes;
    }
    remaining_nodes[current_node_index] = remaining_nodes[--remaining_nodes_count];

    for(int i=0; i<inst->nnodes-1; i++){
        nearest_node_index = euclidean_nearest_node(inst,
                                                    current_node_index,
                                                    remaining_nodes,
                                                    &remaining_nodes_count,
                                                    round);
        printf("%i -> %i \n", current_node_index, nearest_node_index);
        solution[current_node_index] = nearest_node_index;
        current_node_index = nearest_node_index;
    }
}

void tsp_extra_mileage(instance* inst, int* solution, pair starting_pair, bool round, bool verbose, bool randomize){

    if (randomize){
        starting_pair.item1 = rand() % inst->nnodes;
        starting_pair.item2 = rand() % inst->nnodes;
    }

    pair current_pair = starting_pair;

    int remaining_nodes_count = inst->nnodes;
    int remaining_nodes [inst->nnodes];


    // Initialize solution
    for(int i = 0; i < inst->nnodes; i++){
        solution[i] = -1;
        remaining_nodes[i] = i;
    }
    solution[current_pair.item1] = current_pair.item2;
    solution[current_pair.item2] = current_pair.item1;
    remaining_nodes[current_pair.item1] = remaining_nodes[--remaining_nodes_count];
    remaining_nodes[current_pair.item2] = remaining_nodes[--remaining_nodes_count];

    for(int i=0; i<remaining_nodes_count; i++){
        current_distance
    }
}


int euclidean_nearest_node(instance* instance, int node, int* remaining_nodes, int* remaining_nodes_count, bool round){
    double min_distance = 0;
    int nearest_node = -1;
    int nearest_node_index = -1;
    for(int j = 0; j < *remaining_nodes_count; j++){
        if(node == remaining_nodes[j]) continue;
        double distance = euclidean_distance(instance->xcoord[node], instance->ycoord[node], instance->xcoord[remaining_nodes[j]], instance->ycoord[remaining_nodes[j]], round);
        if (nearest_node == -1 || distance < min_distance){
            min_distance = distance;
            nearest_node = remaining_nodes[j];
            nearest_node_index = j;
        }
    }
    if (nearest_node == -1) return -1;
    remaining_nodes[nearest_node_index] = remaining_nodes[--*remaining_nodes_count];
    return nearest_node;
}

double compute_solution_cost(instance* inst, const int* solution){
    double cost = 0;
    for(int i = 0; i < inst->nnodes; i++){
        if (solution[i] == -1) continue;
        cost += euclidean_distance(inst->xcoord[i],
                                   inst->ycoord[i],
                                   inst->xcoord[solution[i]],
                                   inst->ycoord[solution[i]], false);
    }
    return cost;
}