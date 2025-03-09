#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cplex.h>
#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"

void print_solution(instance* inst, int* solution) {
    for (int i = 0; i < inst->nnodes; i++) {
        printf("%d -> %d\n", i, solution[i]);
    }
}

void export_to_gnuplot(instance* inst, int* solution) {
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

int main(int argc, char *argv[]) {
    const char *method = NULL;
    const char *tsp_file = NULL;
    int print_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = argv[++i];
        } else if (strcmp(argv[i], "--print") == 0) {
            print_flag = 1;
        } else {
            tsp_file = argv[i];
        }
    }

    if (method == NULL || tsp_file == NULL) {
        fprintf(stderr, "Usage: %s -m <method> [--print] <tsp_file>\n", argv[0]);
        return 1;
    }

    int VERBOSE = 10000;
    instance inst;
    int error;

    strcpy(inst.input_file, tsp_file);
    read_input(&inst);
    int* solution = (int*)calloc(inst.nnodes, sizeof(int));

    if (strcmp(method, "branch_and_cut") == 0) {
        cplex_tsp_branch_and_cut(&inst, solution, VERBOSE);
    }else if (strcmp(method, "cplex_callback_candidate") == 0) {
        cplex_tsp_callback(&inst, solution, VERBOSE, CPX_CALLBACKCONTEXT_CANDIDATE);
    }else if (strcmp(method, "cplex_callback_hard_fixing") == 0) {
        cplex_tsp_callback(&inst, solution, VERBOSE, CPX_CALLBACKCONTEXT_CANDIDATE);
    }
    else if (strcmp(method, "tsp_greedy") == 0) {

    }
    else {
        fprintf(stderr, "Unknown method: %s\n", method);
        free(solution);
        return 1;
    }

    if (print_flag) {
        export_to_gnuplot(&inst, solution);
        system("gnuplot ./gnuplot_commands.txt");
    }

    free(solution);
    return 0;
}