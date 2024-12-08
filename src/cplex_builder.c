//
// Created by WiZ on 13/11/2024.
//

#include "cplex_builder.h"

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if (!inst->integer_costs) return sqrt(dx * dx + dy * dy);
    int dis = sqrt(dx * dx + dy * dy) + 0.499999999; // nearest integer
    return dis + 0.0;
}

int get_cplex_variable_index(int i, int j, instance *inst) // to be verified
{
    if (i == j) print_error(" i == j in xpos");
    if (i > j) return get_cplex_variable_index(j, i, inst);
    int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
    return pos;
}

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

    free(value);
    free(index);

    free(cname[0]);
    free(cname);

    if (VERBOSE >= 100) CPXwriteprob(env, lp, "model.lp", NULL);
}

int TSPopt(instance *inst) {
    // open CPLEX model
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if (error) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if (error) print_error("CPXcreateprob() error");

    build_model(inst, env, lp);

    // Cplex's parameter setting
    CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
    if (VERBOSE >= 60) CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON); // Cplex output on screen
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
    CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0);
    // ...

    error = CPXmipopt(env, lp);
    if (error) {
        printf("CPX error code %d\n", error);
        print_error("CPXmipopt() error");
    }

    // use the optimal solution found by CPLEX
    inst->solution = (int *) calloc(inst->nnodes, sizeof(int));
    int ncols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(ncols, sizeof(double));
    int cpxgetx_error = CPXgetx(env, lp, xstar, 0, ncols - 1);

    if (cpxgetx_error > 0) print_error("CPXgetx() error");

    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            if (xstar[get_cplex_variable_index(i, j, inst)] > 0.5) {
                printf("  ... x(%3d,%3d) = 1\n", i + 1, j + 1);
                inst->solution[i] = j;
            }
        }
    }
    free(xstar);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return 0; // or an appropriate nonzero error code
}

void find_connected_components(int *solution, int *component_map, int nnodes) {
    component_map = (int *) calloc(nnodes, sizeof(int));

    for (int i = 0; i < nnodes; i++) {
        component_map[i] = i;
    }

    for (int i = 0; i < nnodes; i++) {
        int connected_node = solution[i];
        component_map[connected_node] = component_map[i];
    }
}

void add_bender_constraint(const int *component_map, instance *instance, CPXENVptr env, CPXLPptr lp, int ncomponents) {
    if (ncomponents == 1) return;

    double right_hand_side_value = 1.0;

    for (int k = 0; k < ncomponents; k++) {
        int *index = (int *) calloc(instance->nnodes, sizeof(int));
        double *coefficients = (double *) calloc(instance->nnodes, sizeof(double));
        int non_zero_variables_count = 0;
        char constraint_sense = 'L';
        for (int i = 0; i < instance->nnodes; i++) {
            if (component_map[i] != k) continue;
            right_hand_side_value++; // increase cardinality of the set
            for (int j = i + 1; j < instance->nnodes; j++) {
                if (component_map[j] != k) continue;
                index[non_zero_variables_count] = get_cplex_variable_index(i, j, instance);
                coefficients[non_zero_variables_count] = 1.0;
                non_zero_variables_count++;
            }
        }
        if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &right_hand_side_value, &constraint_sense, NULL, index,
                       coefficients, NULL, NULL)) {
            print_error("CPXaddrows(): error 1");
        }
        free(index);
        free(coefficients);
    }
}
