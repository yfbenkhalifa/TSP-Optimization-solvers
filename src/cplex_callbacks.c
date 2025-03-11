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
    callback_data *data;
    TRY_CATCH_FATAL({
        data = (callback_data*) userhandle;
        });
    double *xstar = (double *) malloc(data->instance->ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    int error = CPXcallbackgetcandidatepoint(context, xstar, 0, data->instance->ncols - 1, &objval);
    if (error) {
        free(xstar);
        print_error("CPXcallbackgetcandidatepoint error");
    }

    double incumbent = CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);

    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(data->instance, &component_map, &succ, &ncomp);
    build_solution(xstar, data->instance, succ, component_map, ncomp);

    if (*ncomp > 1) {
        add_bender_constraint(NULL, NULL, context, component_map, data->instance, *ncomp);
    }

    if (data->hard_fixing_p_fix > 0) {
        error = cplex_hard_fixing(data->instance, context, data->hard_fixing_p_fix);
        if (error) print_error("cplex_hard_fixing error");
    }

    if (data->local_branching_k > 0) {
        add_local_branching_constraint(data->instance, NULL, NULL, context, xstar, data->local_branching_k);
    }

    free(xstar);
    free(component_map);
    free(succ);
    return 0;
}

int CPXPUBLIC callback_function_relaxation(CPXCALLBACKCONTEXTptr context, void *userhandle) {
    instance *inst = (instance *) userhandle;
    double *xstar = (double *) malloc(inst->ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    if (CPXcallbackgetcandidatepoint(context, xstar, 1, inst->ncols, &objval)) print_error(
        "CPXcallbackgetcandidatepoint error");

    double incumbent = CPX_INFBOUND;
    CPXcallbackgetinfodbl(context, CPXCALLBACKINFO_BEST_SOL, &incumbent);
    int *component_map;
    int *succ;
    int *ncomp;
    init_data_struct(inst, &component_map, &succ, &ncomp);
    build_solution(xstar, inst, succ, component_map, ncomp);
    if (*ncomp > 1) {
        add_bender_constraint(NULL, NULL, context, component_map, inst, *ncomp);
    }

    free(xstar);
    free(component_map);
    free(succ);
    return 0;
}
