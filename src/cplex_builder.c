//
// Created by WiZ on 13/11/2024.
//

#include "cplex_builder.h"

#include "utils.h"

#define EPS 1e-5
int VERBOSE = 10000;


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
    int max_iterations_without_improvement = 10;
    double start_time = clock();
    double best_incumbent_cost = CPX_INFBOUND;
    do
    {
        int max_iterations = 1000;
        if (iteration_count > max_iterations && max_iterations >= 0) stop_condition = true;

        double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

        if (elapsed_time > time_limit && time_limit >= 0) stop_condition = true;

        log_message(LOG_LEVEL_INFO, "CPLEX iteration %d\n", iteration_count);

        xstar = TSPopt(instance, env, lp);
        init_data_struct(instance, &component_map, &succ, &ncomp);
        build_solution(xstar, instance, solution, component_map, ncomp);
        double current_incumbent_cost = compute_solution_cost(instance, solution);

        if (current_incumbent_cost < best_incumbent_cost)
        {
            best_incumbent_cost = current_incumbent_cost;
        }else
        {
            iterations_without_improvement++;
        }

        error = add_bender_constraint(env, lp, NULL, component_map, instance, *ncomp);


        if (max_iterations >=0 ) iteration_count++;

        if (iterations_without_improvement > max_iterations_without_improvement
            && max_iterations_without_improvement >= 0) stop_condition = true;
    }
    while (!is_tsp_solution(instance, solution) && !stop_condition);

    instance->best_cost_value = best_incumbent_cost;
    instance->elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

    log_message(LOG_LEVEL_INFO, "TSP solution time: %f seconds\n", instance->elapsed_time);
    log_message(LOG_LEVEL_INFO, "TSP solution cost: %f\n", instance->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Iteration count: %d\n", iteration_count);

    free(component_map);
    free(xstar);


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
    clock_t start_time = clock();

    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");

    // Parameters initialization
    double lower_bound = -CPX_INFBOUND;
    double upper_bound = CPX_INFBOUND;
    int ncols = instance->ncols;
    double best_incumbent_cost = CPX_INFBOUND;
    int iteration_count = 0;

    // CPLEX parameters
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (_verbose >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 1);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 36000);
    CPXsetintparam(env, CPX_PARAM_CUTUP, upper_bound);

    // Build initial model
    build_model(instance, env, lp);

    // Set callback function
    if (contextid == NULL) contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
    if (CPXcallbacksetfunc(env, lp, contextid, callback_driver, instance))
        print_error("CPXcallbacksetfunc() error");

    // Allocate memory for solution tracking
    double* xstar = (double*)calloc(ncols, sizeof(double));
    int* component_map = (int*)calloc(instance->nnodes, sizeof(int));
    int* ncomp = NULL;
    bool stop_condition = false;
    int iterations_without_improvement = 0;
    int max_iterations_without_improvement = 10;
    int max_iterations = 1000;
    do
    {
        log_message(LOG_LEVEL_INFO, "CPLEX iteration %d\n", iteration_count);
        int max_iterations = 1000;
        if (iteration_count > max_iterations && max_iterations >= 0) stop_condition = true;

        xstar = TSPopt(instance, env, lp);
        init_data_struct(instance, &component_map, &solution, &ncomp);
        build_solution(xstar, instance, solution, component_map, ncomp);
        double current_incumbent_cost = compute_solution_cost(instance, solution);

        if (current_incumbent_cost < best_incumbent_cost)
        {
            best_incumbent_cost = current_incumbent_cost;
        }else
        {
            iterations_without_improvement++;
        }

        if (max_iterations >=0 ) iteration_count++;

        if (iterations_without_improvement > max_iterations_without_improvement
            && max_iterations_without_improvement >= 0) stop_condition = true;

    }while (!is_tsp_solution(instance, solution) && !stop_condition);

    instance->best_cost_value = best_incumbent_cost;
    instance->elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;


    log_message(LOG_LEVEL_INFO, "TSP solution time: %f seconds\n", instance->elapsed_time);
    log_message(LOG_LEVEL_INFO, "TSP solution cost: %f\n", instance->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Iteration count: %d\n", iteration_count);

    free(component_map);
    free(xstar);
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return error;
}

void add_local_branching_constraint(instance* inst, CPXENVptr env, CPXLPptr lp, int* solution, int k)
{
    int* index = (int*)calloc(inst->nnodes, sizeof(int));
    double* value = (double*)calloc(inst->nnodes, sizeof(double));
    double rhs = k;
    char sense = 'L'; // 'L' for less than or equal to constraint

    int non_zero_variables_count = 0;
    for (int i = 0; i < inst->nnodes; i++)
    {
        for (int j = i + 1; j < inst->nnodes; j++)
        {
            if (solution[i] == j || solution[j] == i)
            {
                index[non_zero_variables_count] = get_cplex_variable_index(i, j, inst);
                value[non_zero_variables_count] = 1.0;
                non_zero_variables_count++;
            }
        }
    }

    int izero = 0;
    if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &rhs, &sense, &izero, index, value, NULL, NULL))
    {
        print_error("CPXaddrows(): error adding local branching constraint");
    }

    free(index);
    free(value);
}
