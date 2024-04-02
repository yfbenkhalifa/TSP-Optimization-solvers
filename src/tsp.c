//
// Created by wiz on 3/29/24.
//

#include "tsp.h"
#define VERBOSE 50


void tsp_greedy(instance* inst, int starting_node, bool round, bool verbose){
    int solution[inst->nnodes];
    int index = 0;

    // Initialize solution
    for(int i = 0; i < inst->nnodes; i++){
        solution[i] = -1;
    }

    // Add starting node to solution
    solution[index] = starting_node;

    // Search for the nearest node
}

int nearest_node(instance* instance, int node, bool round){
    double min_distance = 0;
    int nearest_node = -1;
    for(int i = 0; i < instance->nnodes; i++){
        if(i != node){
            double distance = euclidean_distance(instance->xcoord[node], instance->ycoord[node], instance->xcoord[i], instance->ycoord[i], round);
            if(nearest_node == -1 || distance < min_distance){
                min_distance = distance;
                nearest_node = i;
            }
        }
    }
    return nearest_node;
}