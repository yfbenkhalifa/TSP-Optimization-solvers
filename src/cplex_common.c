//
// Created by WiZ on 19/01/2025.
//

#include "cplex_common.h"

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if (!inst->integer_costs) return sqrt(dx * dx + dy * dy);
    int dis = sqrt(dx * dx + dy * dy) + 0.499999999;
    return dis + 0.0;
}

int get_cplex_variable_index(int i, int j, instance *inst) // NOLINT(*-no-recursion)
{
    if (i == j) print_error(" i == j in xpos");
    if (i > j) return get_cplex_variable_index(j, i, inst);
    int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
    return pos;
}

void init_data_struct(instance *inst, int **component_map, int **succ, int **ncomp) {
    *component_map = (int *) malloc(inst->nnodes * sizeof(int));
    *succ = (int *) malloc(inst->nnodes * sizeof(int));
    *ncomp = (int *) malloc(sizeof(int));
}

void build_solution(double *xstar, instance *instance, int *succ, int *comp, int *ncomp) {
#ifdef  DEBUG

    int *degree = (int *) calloc(instance->nnodes, sizeof(int));
    for (int i=0; i < instance->nnodes; i++) {
        for (int j = i+1 ; j < instance->nnodes; j++) {
            int xpos = get_cplex_variable_index(i, j, instance)
            if (fabs(xstar[xpos]) > EPS && fabs(xstar[xpos]-1.0) > EPS) print_error("ERROR: invalid value for xpos")

            if (xstar[xpos] > 0.5) {
                ++degree[i];
                ++degree[j];
            }
        }
    }
    for (int i=0; i < instance->nnodes; i++) {
        if (degree[i] != 2) print_error("ERROR: invalid solution")
    }

#endif

    *ncomp = 0;
    for (int i = 0; i < instance->nnodes; i++) {
        succ[i] = -1;
        comp[i] = -1;
    }

    for (int start = 0; start < instance->nnodes; start++) {
        if (comp[start] >= 0) continue;

        (*ncomp)++;
        int i = start;
        bool done = false;
        while (!done) {
            comp[i] = *ncomp;
            done = true;
            for (int j = 0; j < instance->nnodes; j++) {
                if (i != j && xstar[get_cplex_variable_index(i, j, instance)] > 0.5 && comp[j] == -1) {
                    succ[i] = j;
                    i = j;
                    done = false;
                    break;
                }
            }
        }
        succ[i] = start;
    }
}


int add_bender_constraint(CPXENVptr env, CPXLPptr lp, CPXCALLBACKCONTEXTptr context, const int *component_map,
                          instance *instance, int ncomponents) {
    if (ncomponents == 1) return 0;

    double right_hand_side_value = 0.0;
    int ncols = instance->ncols;
    int *index = (int *) calloc(ncols, sizeof(int));
    double *coefficients = (double *) calloc(ncols, sizeof(double));
    int izero = 0;
    for (int k = 1; k <= ncomponents; k++) {
        int non_zero_variables_count = 0;
        right_hand_side_value = 0.0;
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
        right_hand_side_value--;
        if (right_hand_side_value >= instance->nnodes) continue;
        if (env != NULL && lp != NULL) {
            if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &right_hand_side_value, &constraint_sense, &izero,
                           index,
                           coefficients, NULL, NULL)) {
                print_error("CPXaddrows(): error 1");
                return 1;
                           }
        } else if (context != NULL) {
            if (CPXcallbackrejectcandidate(context, 1, non_zero_variables_count, &right_hand_side_value,
                                           &constraint_sense, &izero, index, coefficients)) {
                print_error("CPXcallbackrejectcandidate() error");
                return 1;
                                           }
        }
    }

    free(index);
    free(coefficients);
    return 0;
}