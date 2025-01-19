//
// Created by WiZ on 13/11/2024.
//

#include "cplex_builder.h"

#define EPS 1e-5
int VERBOSE = 10000;


void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    double zero = 0.0;
    char binary = 'B';

    char **cname = (char **) calloc(1, sizeof(char *)); // (char **) required by cplex...
    cname[0] = (char *) calloc(100, sizeof(char));

    // add binary var.s x(i,j) for i < j

    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            sprintf(cname[0], "x(%d,%d)", i + 1, j + 1); // ... x(1,2), x(1,3) ....
            double cost_function_value = dist(i, j, inst); // cost == distance
            double lower_bound = 0.0;
            double upper_bound = 1.0;
            if (CPXnewcols(env, lp, 1, &cost_function_value, &lower_bound, &upper_bound, &binary, cname)) {
                print_error(" wrong CPXnewcols on x var.s");
            }
            if (CPXgetnumcols(env, lp) - 1 != get_cplex_variable_index(i, j, inst)) {
                print_error(" wrong position for x var.s");
            }
        }
    }

    int *index = (int *) calloc(inst->nnodes, sizeof(int));
    double *value = (double *) calloc(inst->nnodes, sizeof(double));

    for (int h = 0; h < inst->nnodes; h++) // add the degree constraint on node h
    {
        double rhs = 2.0;
        char sense = 'E'; // 'E' for equality constraint
        sprintf(cname[0], "degree(%d)", h + 1);
        int non_zero_variables_count = 0;
        for (int i = 0; i < inst->nnodes; i++) {
            if (i == h) continue;
            index[non_zero_variables_count] = get_cplex_variable_index(i, h, inst);
            value[non_zero_variables_count] = 1.0;
            non_zero_variables_count++;
        }
        int izero = 0;
        if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &rhs, &sense, &izero, index, value, NULL, &cname[0])) {
            print_error("CPXaddrows(): error 1");
        }
    }

    inst->ncols = CPXgetnumcols(env, lp);

    free(value);
    free(index);

    free(cname[0]);
    free(cname);

    if (VERBOSE >= 100) CPXwriteprob(env, lp, "model.lp", NULL);
}

double *TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp) {
    int error;
    error = CPXmipopt(env, lp);
    if (error) {
        printf("CPX error code %d\n", error);
        print_error("CPXmipopt() error");
    }

    int ncols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(ncols, sizeof(double));
    int cpxgetx_error = CPXgetx(env, lp, xstar, 0, ncols - 1);

    if (cpxgetx_error > 0) print_error("CPXgetx() error");

    return xstar;
}


int cplex_tsp_branch_and_cut(instance *instance, int *solution, int _verbose) {
    int error = 0;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");
    // Cplex's parameter setting
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (_verbose >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 4);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 3600); // time limit
    CPXsetintparam(env, CPX_PARAM_CUTUP, CPX_INFBOUND); // disable the cut-off
    build_model(instance, env, lp);
    double *xstar;
    int *component_map;
    int *succ;
    int *ncomp;


    do {
        xstar = TSPopt(instance, env, lp);
        init_data_struct(instance, &component_map, &succ, &ncomp);
        build_solution(xstar, instance, solution, component_map, ncomp);
        error = add_bender_constraint(env, lp, NULL, component_map, instance, *ncomp);
    } while (*ncomp > 1);

    instance->best_cost_value = compute_solution_cost(instance, solution);

    free(component_map);
    free(xstar);


    return error;
}

int cplex_tsp_callback(instance *instance, int *solution, int _verbose, CPXLONG contextid) {
    int error = 0;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");
    double lower_bound = -CPX_INFBOUND;
    double upper_bound = CPX_INFBOUND;

    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (_verbose >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 43);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 3600);
    CPXsetintparam(env, CPX_PARAM_CUTUP, upper_bound);

    build_model(instance, env, lp);
    if (contextid == NULL) contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
    if (CPXcallbacksetfunc(env, lp, contextid, callback_driver, instance)) print_error("CPXcallbacksetfunc() error");
    error = CPXmipopt(env, lp);

    CPXgetobjval(env, lp, &upper_bound);
    CPXgetbestobjval(env, lp, &lower_bound);

    instance->best_cost_value = compute_solution_cost(instance, solution);

    return error;
}
