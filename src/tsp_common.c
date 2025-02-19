//
// Created by WiZ on 20/11/2024.
//

#include "tsp_common.h"
#include "utils.h"

void print_error(const char* err) { printf("\n\n ERROR: %s \n\n", err); }

double compute_solution_cost(const instance* inst, const int* solution)
{
    double cost = 0;
    for (int i = 0; i < inst->nnodes; i++)
    {
        if (solution[i] == -1) continue;
        cost += euclidean_distance(inst->xcoord[i],
                                   inst->ycoord[i],
                                   inst->xcoord[solution[i]],
                                   inst->ycoord[solution[i]], false);
    }
    return cost;
}

bool is_tsp_solution(instance* inst, int* solution)
{
    for (int i = 0; i < inst->nnodes; i++)
    {
        if (solution[i] == -1) return false;
    }
    if (has_duplicates(inst, solution)) return false;
    if (!is_acyclic(inst, solution)) return false;
    return true;
}
