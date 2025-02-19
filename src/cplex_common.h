//
// Created by WiZ on 19/01/2025.
//

#ifndef CPLEX_COMMON_H
#define CPLEX_COMMON_H
#include "common.h"
#include <cplex.h>
#include "tsp_common.h"


typedef enum{
    CPLEX_SOLVE_BRANCH_AND_CUT,
    CPLEX_SOLVE_CALLBACK
}CplexSolveType;

void build_solution(double *xstar, instance *instance, int *succ, int *comp, int *ncomp);
int add_bender_constraint(CPXENVptr env, CPXLPptr lp, CPXCALLBACKCONTEXTptr context, const int *component_map,
                          instance *instance, int ncomponents);
int get_cplex_variable_index(int i, int j, instance *inst);
double dist(int i, int j, instance *inst);
void init_data_struct(instance *inst, int **component_map, int **succ, int **ncomp);

#endif //CPLEX_COMMON_H