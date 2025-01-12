#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include <string.h>
#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"


void init_data_struct(instance inst, int **component_map, int **succ, int **ncomp) {
    *component_map = (int *)malloc(inst.nnodes * sizeof(int));
    *succ = (int *)malloc(inst.nnodes * sizeof(int));
    *ncomp = (int *)malloc(sizeof(int));
}

int main() {
    int VERBOSE = 10000;
    instance inst;
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");
    // Cplex's parameter setting
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (VERBOSE >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 4);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 3600); // time limit
    CPXsetintparam(env, CPX_PARAM_CUTUP, CPX_INFBOUND); // disable the cut-off

    strcpy(inst.input_file, "../data/att48.tsp");
    read_input(&inst);
    build_model(&inst, env, lp);
    double* xstar = TSPopt(&inst, env, lp);
    CPXwriteprob(env, lp, "model.lp", NULL);
    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(inst, &component_map, &succ, &ncomp);

    build_solution(xstar, &inst, succ, component_map, ncomp);
    //
    add_bender_constraint(component_map, &inst, env, lp, *ncomp);
    //
    CPXwriteprob(env, lp, "model_2", "lp");
    //
    xstar = TSPopt(&inst, env, lp);

    init_data_struct(inst, &component_map, &succ, &ncomp);

    build_solution(xstar, &inst, succ, component_map, ncomp);

    add_bender_constraint(component_map, &inst, env, lp, *ncomp);

    CPXwriteprob(env, lp, "model_3.lp", NULL);

    xstar = TSPopt(&inst, env, lp);



    free(component_map);
    free(xstar);
    free(inst.solution);
    return 0;
}



