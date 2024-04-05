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
        nearest_node_index = nearest_node(inst, current_node_index, remaining_nodes, &remaining_nodes_count, false);
        if (nearest_node_index == -1) break;
        printf("%i -> %i \n", current_node_index, nearest_node_index);
        solution[current_node_index] = nearest_node_index;
        current_node_index = nearest_node_index;
    }
}

int nearest_node(instance* instance, int node, int* remaining_nodes, int* remaining_nodes_count, bool round){
    double min_distance = 0;
    int nearest_node = -1;
    for(int i = 0; i < *remaining_nodes_count; i++){
        if(node == remaining_nodes[i]) continue;
        double distance = euclidean_distance(instance->xcoord[node], instance->ycoord[node], instance->xcoord[remaining_nodes[i]], instance->ycoord[remaining_nodes[i]], round);
        if (nearest_node == -1 || distance < min_distance){
            min_distance = distance;
            nearest_node = remaining_nodes[i];
        }
    }
    if (nearest_node == -1) return -1;
    remaining_nodes[nearest_node] = remaining_nodes[--*remaining_nodes_count];
    return nearest_node;
}