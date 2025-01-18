#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include <string.h>
#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"


void print_solution(instance* inst, int* solution)
{
    for (int i = 0; i < inst->nnodes; i++)
    {
        printf("%d -> %d\n", i, solution[i]);
    }
}

void export_to_gnuplot(instance* inst, int* solution)
{
    FILE *fout = fopen("./data.dat", "w");
    if (fout == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < inst->nnodes; i++) {
        int next_node = solution[i];
        fprintf(fout, "%f %f %d\n", inst->xcoord[i], inst->ycoord[i], i);
        fprintf(fout, "%f %f %d\n\n", inst->xcoord[next_node], inst->ycoord[next_node], next_node);
    }

    fclose(fout);
}

int main()
{
    int VERBOSE = 10000;
    instance inst;
    int error;

    strcpy(inst.input_file, "../data/tsp_mock.tsp");
    read_input(&inst);
    int* solution = (int*)calloc(inst.nnodes, sizeof(int));
    error = cplex_tsp_branch_and_cut(&inst, solution, VERBOSE);
    export_to_gnuplot(&inst, solution);
    print_solution(&inst, solution);
    system("gnuplot ./gnuplot_commands.txt");
    return 0;
}
