//
// Created by wiz on 3/19/24.
//

#ifndef OR2PROJECT_UTILS_H
#define OR2PROJECT_UTILS_H

#include <math.h>
#include "common.h"
#include "tsp.h"
#include "tsp_common.h"

typedef struct {
    char *instance_name;
    char **methods;
    double **costs;
    int methods_count;
    int instances_count;
} profiler_run;

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

void log_message(LogLevel level, const char *format, ...);

void log_results_to_csv(const char *filename, const char *instance, const char *method_name, double best_cost,
                        double elapsed_time);

void read_input(instance *inst);

void parse_command_line(int argc, char **argv, instance *inst);

void free_instance(instance *inst);

double euclidean_distance(double x1, double y1, double x2, double y2, bool round);

bool has_duplicates(instance *inst, const int *solution);

bool is_acyclic(instance *inst, const int *solution);

instance generate_instance(int nnodes, double *demand, double *xcoord, double *ycoord, int depot, double capacity,
                           int nveh);

bool compare_solutions(const int *solution1, const int *solution2, int size);

void export_solution(const char *filename, instance *inst);

void generate_random_instance(instance *inst, int nnodes, int seed);
#endif //OR2PROJECT_UTILS_H
