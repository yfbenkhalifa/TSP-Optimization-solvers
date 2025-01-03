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
    instance inst;
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");

    strcpy(inst.input_file, "../data/tsp_mock.tsp");
    read_input(&inst);
    double* xstar = TSPopt(&inst, env, lp);
    export_solution("../test.txt", &inst);

    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(inst, &component_map, &succ, &ncomp);

    build_solution(xstar, &inst, succ, component_map, ncomp);

    add_bender_constraint(component_map, &inst, env, lp, *ncomp);


    free(component_map);
    free(xstar);
    free(inst.solution);
    return 0;
}



