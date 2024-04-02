//
// Created by wiz on 3/19/24.
//

#ifndef OR2PROJECT_UTILS_H
#define OR2PROJECT_UTILS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cplex.h>
#include <pthread.h>
#endif //OR2PROJECT_UTILS_H

typedef struct{
    int n;
    int x[1000];
    int y[1000];
}gnuplot_instance;

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

    // model;
    int xstart;
    int qstart;
    int bigqstart;
    int sstart;
    int bigsstart;
    int ystart;
    int fstart;
    int zstart;
} instance;


void print_error(const char *err);
double euclidean_distance(double x1, double y1, double x2, double y2, bool round);
void read_input(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void free_instance(instance *inst);
gnuplot_instance instance_to_gnuplot(instance *inst);

