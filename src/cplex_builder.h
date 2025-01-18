// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include <cplex.h>
#include "tsp_common.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double* TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp);
double dist(int i, int j, instance *inst);
void build_solution(double *xstar, instance *instance, int *succ, int *comp, int *ncomp);
int get_cplex_variable_index(int i, int j, instance* inst);
void findConnectedComponentsFromSolution(int *solution, int nnodes, int *component_map);
int add_bender_constraint(const int* component_map, instance* instance, CPXENVptr env, CPXLPptr lp, int ncomponents);
void add_bender_constraint_from_component(int *component, instance *instance, CPXENVptr env, CPXLPptr lp, int component_length);
int cplex_tsp_branch_and_cut(instance *instance,  int *solution, int _verbose);
#endif // CPLEX_BUILDER_H