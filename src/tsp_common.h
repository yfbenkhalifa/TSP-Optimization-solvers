//
// Created by WiZ on 20/11/2024.
//

#ifndef TSP_COMMON_H
#define TSP_COMMON_H
#include "common.h"
typedef struct{
    double item1;
    double item2;
} tuple;

typedef struct{
    int node;
    int node_index;
    double cost;
}solution_node;


typedef struct{
    // node1 -> node2
    int node1;
    int node2;
}pair;

typedef struct {

    // metadata
    double best_known_solution_value;

    //input data
    int nnodes;
    double *demand;
    double *xcoord;
    double *ycoord;
    int depot;
    double capacity;
    int nveh;

    // parameters
    int model_type;
    int old_benders;
    int randomseed;
    int num_threads;
    double timelimit;						// overall time limit, in sec.s
    char input_file[1000];		  			// input file
    char node_file[1000];		  			// cplex node file
    int available_memory;
    int max_nodes; 							// max n. of branching nodes in the final run (-1 unlimited)
    double cutoff; 							// cutoff (upper bound) for master
    int integer_costs;

    //global data
    double	tstart;
    double zbest;							// best sol. available
    double tbest;							// time for the best sol. available
    double *best_sol;						// best sol. available
    double	best_lb;						// best lower bound available
    double *load_min;						// minimum load when leaving a node
    double *load_max;						// maximum load when leaving a node

    //solution data
    int* solution;
    double best_cost_value;

    // model;
    int xstart;
    int qstart;
    int bigqstart;
    int sstart;
    int bigsstart;
    int ystart;
    int fstart;
    int zstart;
    int ncols;
    double elapsed_time;
} instance;

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
    double cost;
} Solution;

void print_error(const char *err);
double compute_solution_cost(const instance *instance, const int *solution);
bool is_tsp_solution(instance* inst, int* solution);
#endif //TSP_COMMON_H
