// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include <cplex.h>
#include "tsp_common.h"

typedef struct {
    int nnodes;
    int* solution;
}cplex_tsp_solution;

typedef struct {
    int error_code;
    int* solution;
}cplex_tsp_result;

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
int TSPopt(instance *inst);
void find_connected_components(int* solution, int* connected_components, int nnodes);
void add_bender_constraint(int component_index, int* solution, int nnodes, CPXENVptr env, CPXLPptr lp);

#endif // CPLEX_BUILDER_H