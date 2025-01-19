// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include "cplex_common.h"
#include "cplex_callbacks.h"
#include "tsp_common.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double* TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp);
int cplex_tsp_branch_and_cut(instance *instance,  int *solution, int _verbose);
int cplex_tsp_callback(instance *instance, int *solution, int _verbose, CPXLONG contextid);
#endif // CPLEX_BUILDER_H