#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include <string.h>

#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"


int main() {
    instance inst;
    strcpy(inst.input_file, "../data/tsp_mock.tsp");
    read_input(&inst);
    TSPopt(&inst);

    export_solution("../test.txt", &inst);

    // connected components
    int* connected_components = (int*) calloc(inst.nnodes, sizeof(int));
    find_connected_components(inst.solution, connected_components, inst.nnodes);

    for (int i=0; i<inst.nnodes; i++) {
        printf("%d -> %d | ", i, inst.solution[i]);
    }

    for (int i=0; i<inst.nnodes; i++) {
        printf("Node %d is in component %d\n", i, connected_components[i]);
    }
}
