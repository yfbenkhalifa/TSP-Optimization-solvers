//
// Created by WiZ on 13/11/2024.
//

#include "cplex_builder.h"

#include "utils.h"

#define EPS 1e-5
int VERBOSE = 10000;

void print_cplex_model(CPXENVptr env, CPXLPptr lp, const char* filename)
{
    int status = CPXwriteprob(env, lp, filename, NULL);
    if (status)
    {
        fprintf(stderr, "Failed to write CPLEX model to file: %s\n", filename);
    }
    else
    {
        printf("CPLEX model written to file: %s\n", filename);
    }
}


void build_model(instance* inst, CPXENVptr env, CPXLPptr lp)
{
    double zero = 0.0;
    char binary = 'B';

    char** cname = (char**)calloc(1, sizeof(char*));
    cname[0] = (char*)calloc(100, sizeof(char));

    for (int i = 0; i < inst->nnodes; i++)
    {
        for (int j = i + 1; j < inst->nnodes; j++)
        {
            sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);
            double cost_function_value = dist(i, j, inst);
            double lower_bound = 0.0;
            double upper_bound = 1.0;
            if (CPXnewcols(env, lp, 1, &cost_function_value, &lower_bound, &upper_bound, &binary, cname))
            {
                print_error(" wrong CPXnewcols on x var.s");
            }
            if (CPXgetnumcols(env, lp) - 1 != get_cplex_variable_index(i, j, inst))
            {
                print_error(" wrong position for x var.s");
            }
        }
    }

    int* index = (int*)calloc(inst->nnodes, sizeof(int));
    double* value = (double*)calloc(inst->nnodes, sizeof(double));

    for (int h = 0; h < inst->nnodes; h++) // add the degree constraint on node h
    {
        double rhs = 2.0;
        char sense = 'E'; // 'E' for equality constraint
        sprintf(cname[0], "degree(%d)", h + 1);
        int non_zero_variables_count = 0;
        for (int i = 0; i < inst->nnodes; i++)
        {
            if (i == h) continue;
            index[non_zero_variables_count] = get_cplex_variable_index(i, h, inst);
            value[non_zero_variables_count] = 1.0;
            non_zero_variables_count++;
        }
        int izero = 0;
        if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &rhs, &sense, &izero, index, value, NULL, &cname[0]))
        {
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

double* TSPopt(instance* inst, CPXENVptr env, CPXLPptr lp)
{
    int error;
    error = CPXmipopt(env, lp);
    if (error)
    {
        printf("CPX error code %d\n", error);
        print_error("CPXmipopt() error");
    }

    int ncols = CPXgetnumcols(env, lp);
    double* xstar = (double*)calloc(ncols, sizeof(double));
    int cpxgetx_error = CPXgetx(env, lp, xstar, 0, ncols - 1);

    if (cpxgetx_error > 0) print_error("CPXgetx() error");

    return xstar;
}

int cplex_tsp_branch_and_cut(instance* instance, int* solution, int _verbose)
{
    int error = 0;
    const double time_limit = -1;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (_verbose >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 43);
    CPXsetdblparam(env, CPX_PARAM_TILIM, time_limit); // time limit
    CPXsetintparam(env, CPX_PARAM_CUTUP, CPX_INFBOUND); // disable the cut-off
    build_model(instance, env, lp);
    double* xstar;
    int* component_map;
    int* succ;
    int* ncomp;
    bool stop_condition = false;
    int iteration_count = 0;
    int iterations_without_improvement = 0;
    int max_iterations_without_improvement = 100;
    double start_time = clock();
    double best_incumbent_cost = CPX_INFBOUND;
    int constraint_idx = -1;
    int max_iterations = 10000;
    do
    {

        if (iteration_count > max_iterations && max_iterations >= 0) stop_condition = true;

        double elapsed_time = (clock() - start_time) / CLOCKS_PER_SEC;

        if (elapsed_time > time_limit && time_limit >= 0) stop_condition = true;

        log_message(LOG_LEVEL_INFO, "CPLEX iteration %d\n", iteration_count);

        xstar = TSPopt(instance, env, lp);
        init_data_struct(instance, &component_map, &succ, &ncomp);
        build_solution(xstar, instance, solution, component_map, ncomp);
        double current_incumbent_cost = compute_solution_cost(instance, solution);

        if (current_incumbent_cost < best_incumbent_cost)
        {
            best_incumbent_cost = current_incumbent_cost;
        }
        else
        {
            iterations_without_improvement++;
        }

        error = add_bender_constraint(env, lp, NULL, component_map, instance, *ncomp);

        if (constraint_idx >= 0)
        {
            CPXdelrows(env, lp, constraint_idx, constraint_idx);
        }

        constraint_idx = add_local_branching_constraint(instance, env, lp, NULL, xstar, 20);


        if (max_iterations >= 0) iteration_count++;

        if (iterations_without_improvement > max_iterations_without_improvement
            && max_iterations_without_improvement >= 0)
            stop_condition = true;
    }
    while (!is_tsp_solution(instance, solution) || stop_condition || *ncomp > 1);

    instance->best_cost_value = best_incumbent_cost;
    instance->elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

    log_message(LOG_LEVEL_INFO, "TSP solution time: %f seconds\n", instance->elapsed_time);
    log_message(LOG_LEVEL_INFO, "TSP solution cost: %f\n", instance->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Iteration count: %d\n", iteration_count);

    print_cplex_model(env, lp, "model_branch_and_cut.lp");

    free(component_map);
    free(xstar);
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return error;
}

int cplex_hard_fixing(instance* instance, CPXCALLBACKCONTEXTptr context, double p_fix)
{
    int ncols = instance->ncols;
    double* xstar = (double*)calloc(ncols, sizeof(double));
    int* indices = (int*)calloc(ncols, sizeof(int));
    double* bd = (double*)calloc(ncols, sizeof(double));
    char* lu = (char*)calloc(ncols, sizeof(char));

    // Initialize random seed
    srand(time(NULL));

    // Get the current solution
    int error = CPXcallbackgetcandidatepoint(context, xstar, 0, ncols - 1, NULL);
    if (error)
    {
        free(xstar);
        free(indices);
        free(bd);
        free(lu);
        return error;
    }

    // Reset the bounds of all variables to their original values (0.0 to 1.0)
    for (int i = 0; i < ncols; i++)
    {
        indices[i] = i;
        lu[i] = 'B'; // Both lower and upper bounds
        bd[i] = 0.0; // Lower bound
    }
    error = CPXcallbackpostheursoln(context, ncols, indices, bd, 0.0, CPXCALLBACKSOLUTION_PROPAGATE);
    if (error)
    {
        free(xstar);
        free(indices);
        free(bd);
        free(lu);
        return error;
    }

    for (int i = 0; i < ncols; i++)
    {
        bd[i] = 1.0; // Upper bound
    }

    error = CPXcallbackpostheursoln(context, ncols, indices, bd, 0.0, CPXCALLBACKSOLUTION_PROPAGATE);
    if (error)
    {
        free(xstar);
        free(indices);
        free(bd);
        free(lu);
        return error;
    }

    // Fix a subset of variables based on the given probability
    int fix_count = 0;
    for (int i = 0; i < ncols; i++)
    {
        if (((double)rand() / RAND_MAX) < p_fix)
        {
            indices[fix_count] = i;
            lu[fix_count] = 'L'; // Only lower bound
            bd[fix_count] = 1.0; // Fix to 1
            fix_count++;
        }
    }

    // Apply the fixing
    if (fix_count > 0)
    {
        error = CPXcallbackpostheursoln(context, fix_count, indices, bd, 0.0, CPXCALLBACKSOLUTION_PROPAGATE);
        if (error)
        {
            free(xstar);
            free(indices);
            free(bd);
            free(lu);
            return error;
        }
    }

    free(xstar);
    free(indices);
    free(bd);
    free(lu);
    return error;
}

int cplex_tsp_callback(instance* instance, int* solution, int _verbose, CPXLONG contextid)
{
    int error = 0;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");

    // TODO: Init starting solution with lower bound update
    double lower_bound = -CPX_INFBOUND;
    double upper_bound = CPX_INFBOUND;
    int ncols = instance->ncols;
    double best_incumbent_cost = CPX_INFBOUND;
    int iteration_count = 0;

    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (_verbose >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 1);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 36000);
    CPXsetintparam(env, CPX_PARAM_CUTUP, upper_bound);

    // Build initial model
    build_model(instance, env, lp);

    // Set callback function
    if (contextid == NULL) contextid = CPX_CALLBACKCONTEXT_CANDIDATE;

    callback_data* cb_data;

    TRY_CATCH_FATAL({
        cb_data = (callback_data*) malloc(sizeof(callback_data));}
    );

    if (!cb_data) print_error("Failed to allocate callback data");

    cb_data->instance = instance;
    cb_data->hard_fixing_p_fix = 0.3;
    cb_data->local_branching_k = 10;

    if (CPXcallbacksetfunc(env, lp, contextid, callback_driver, cb_data))
        print_error("CPXcallbacksetfunc() error");

    double* xstar = ARRAY_ALLOC(instance->ncols, sizeof(double));
    int* component_map = ARRAY_ALLOC(instance->nnodes, sizeof(int));
    int* succ;
    int* ncomp = NULL;

    xstar = TSPopt(instance, env, lp);
    init_data_struct(instance, &component_map, &succ, &ncomp);
    build_solution(xstar, instance, solution, component_map, ncomp);
    double current_incumbent_cost = compute_solution_cost(instance, solution);

    if (!is_tsp_solution(instance, solution))
    {
        log_message(LOG_LEVEL_ERROR, "Solution is not a valid TSP solution\n");
    }

    instance->best_cost_value = current_incumbent_cost;

    log_message(LOG_LEVEL_INFO, "TSP solution time: %f seconds\n", instance->elapsed_time);
    log_message(LOG_LEVEL_INFO, "TSP solution cost: %f\n", instance->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Iteration count: %d\n", iteration_count);

    print_cplex_model(env, lp, "model_callback.lp");

    free(component_map);
    free(xstar);
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return error;
}

int add_local_branching_constraint(instance* inst, CPXENVptr env, CPXLPptr lp,
                                   CPXCALLBACKCONTEXTptr cpxcallbackcontex_tptr, const double* xstar, double k)
{
    int ncols = CPXgetnumcols(env, lp);
    int* index = (int*)calloc(ncols, sizeof(int));
    double* coefficients = (double*)calloc(ncols, sizeof(double));
    double right_hand_side_value = k;
    char constraint_sense = 'G';
    char* rname = (char*)calloc(100, sizeof(char));
    sprintf(rname, "local_branching_constraint");

    int non_zero_variables_count = 0;
    for (int i = 0; i < ncols; i++)
    {
        if (xstar[i] > 0.8)
        {
            index[non_zero_variables_count] = i;
            coefficients[non_zero_variables_count] = 1.0;
            non_zero_variables_count++;
        }
    }

    right_hand_side_value = inst->nnodes - k;

    int izero = 0;
    int row_index = -1;
    if (cpxcallbackcontex_tptr != NULL)
    {
        int error = CPXcallbackrejectcandidate(cpxcallbackcontex_tptr, 1, non_zero_variables_count,
                                               &right_hand_side_value,
                                               &constraint_sense, &izero, index, coefficients);
        if (error)
        {
            log_message(LOG_LEVEL_ERROR, "CPXcallbackrejectcandidate error %d\n", error);
        }

    }
    else if (env != NULL && lp != NULL)
    {
        int error = CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &right_hand_side_value, &constraint_sense,
                               &izero,
                               index,
                               coefficients, NULL, &rname);
        if (error)
        {
            log_message(LOG_LEVEL_ERROR, "CPXcallbackrejectcandidate error %d\n", error);
        }else
        {
            row_index = CPXgetnumrows(env, lp) - 1;
        }

    }
    else
    {
        log_message(LOG_LEVEL_ERROR,
                    "At least one between the CPLEX Callback context or the CPLEX Environment must be specified");
        log_message(LOG_LEVEL_WARNING, "No constraint could be added");
    }

    free(index);
    free(coefficients);

    return row_index;
}
