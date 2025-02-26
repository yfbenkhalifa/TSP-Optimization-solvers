// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include "cplex_common.h"
#include "cplex_callbacks.h"
#include "tsp_common.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double* TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp);
int cplex_hard_fixing(instance* instance, CPXCALLBACKCONTEXTptr context, double pfix);
int cplex_tsp_branch_and_cut(instance *instance,  int *solution, int _verbose);
int cplex_tsp_callback(instance *instance, int *solution, int _verbose, CPXLONG contextid);
void add_local_branching_constraint(instance* inst, CPXENVptr env, CPXLPptr lp, const double* xstar, double k);
#endif // CPLEX_BUILDER_H