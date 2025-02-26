//
// Created by WiZ on 18/01/2025.
//

#include "cplex_callbacks.h"

#include "cplex_builder.h"


int CPXPUBLIC callback_driver(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle) {
    if (contextid == CPX_CALLBACKCONTEXT_CANDIDATE) {
        return callback_function_candidate(context, userhandle);
    }
    if (contextid == CPX_CALLBACKCONTEXT_RELAXATION) {
        return callback_function_relaxation(context, userhandle);
    }

    return 1;

}

int CPXPUBLIC callback_function_candidate(CPXCALLBACKCONTEXTptr context, void *userhandle) {
    instance* inst = (instance*) userhandle;
    double* xstar = (double*) malloc(inst->ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    int error = CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols-1, &objval);
    if ( error )
    {
        free(xstar);
        print_error("CPXcallbackgetcandidatepoint error");
    }

    double incumbent = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    double p_fix = 0.3;
    if (p_fix > 0)
    {
        error = cplex_hard_fixing(inst, context, p_fix);
    }

    if ( error ) print_error("cplex_hard_fixing error");

    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(inst, &component_map, &succ, &ncomp);
    build_solution(xstar, inst, succ, component_map, ncomp);

    if ( *ncomp > 1)
    {
        add_bender_constraint(NULL, NULL, context, component_map, inst, *ncomp);
    }

    free(xstar);
    free(component_map);
    free(succ);
    return 0;
}

int CPXPUBLIC callback_function_relaxation(CPXCALLBACKCONTEXTptr context, void *userhandle) {
    instance* inst = (instance*) userhandle;
    double* xstar = (double*) malloc(inst->ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    if ( CPXcallbackgetcandidatepoint(context, xstar, 1, inst->ncols, &objval) ) print_error("CPXcallbackgetcandidatepoint error");

    double incumbent = CPX_INFBOUND; CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    // if ( VERBOSE >= 100 ) printf(" ... callback at node %5d thread %2d incumbent %10.2lf, candidate value %10.2lf\n", .....);
    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(inst, &component_map, &succ, &ncomp);
    build_solution(xstar, inst, succ, component_map, ncomp);
    if ( *ncomp > 1)
    {
        int izero = 0;
        int purgeable = CPX_USECUT_FILTER;
        int local = 0;

        add_bender_constraint(NULL, NULL, context, component_map, inst, *ncomp);
    }

    free(xstar);
    free(component_map);
    free(succ);
    return 0;
}


