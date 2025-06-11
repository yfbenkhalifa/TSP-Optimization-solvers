#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cplex.h>
#include <time.h>
#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"

void print_solution(instance *inst, int *solution) {
    for (int i = 0; i < inst->nnodes; i++) {
        printf("%d -> %d\n", i, solution[i]);
    }
}

void export_to_gnuplot(instance *inst, int *solution) {
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
    int random_size = 0;
    const char *log_file = "logfile.log";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = argv[++i];
        } else if (strcmp(argv[i], "--print") == 0) {
            print_flag = 1;
        } else if (strcmp(argv[i], "--random") == 0 && i + 1 < argc) {
            random_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log_file = argv[++i];
        } else {
            tsp_file = argv[i];
        }
    }

    if (method == NULL || (tsp_file == NULL && random_size == 0)) {
        fprintf(stderr, "Usage: %s -m <method> [--print] [--random <size>] [--log <logfile>] <tsp_file>\n", argv[0]);
        return 1;
    }

    int VERBOSE = 10000;
    instance inst;
    memset(&inst, 0, sizeof(instance));

    if (random_size > 0) {
        inst.nnodes = random_size;
        inst.xcoord = (double *) malloc(sizeof(double) * random_size);
        inst.ycoord = (double *) malloc(sizeof(double) * random_size);
        srand((unsigned int) time(NULL));
        for (int i = 0; i < random_size; i++) {
            inst.xcoord[i] = ((double) rand() / RAND_MAX) * 1000.0;
            inst.ycoord[i] = ((double) rand() / RAND_MAX) * 1000.0;
        }
        snprintf(inst.input_file, sizeof(inst.input_file), "random_%d", random_size);
    } else {
        strcpy(inst.input_file, tsp_file);
        read_input(&inst);
    }
    int *solution = (int *) calloc(inst.nnodes, sizeof(int));
    clock_t start = clock();
    if (strcmp(method, "branch_and_cut") == 0) {
        int result = cplex_tsp_branch_and_cut(&inst, solution, VERBOSE, 17);
        if (result != 0) {
            fprintf(stderr, "Error in branch and cut method\n");
            free(solution);
            return result;
        }
    } else if (strcmp(method, "cplex_callback_candidate") == 0) {
        cplex_tsp_callback(&inst, solution, VERBOSE, CPX_CALLBACKCONTEXT_CANDIDATE);
    } else if (strcmp(method, "cplex_callback_hard_fixing") == 0) {
        cplex_tsp_callback(&inst, solution, VERBOSE, CPX_CALLBACKCONTEXT_CANDIDATE);
    } else if (strcmp(method, "tsp_greedy") == 0) {
        // TODO: implement greedy
    } else {
        fprintf(stderr, "Unknown method: %s\n", method);
        free(solution);
        return 1;
    }
    clock_t end = clock();
    double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
    double cost = 0.0;
    for (int i = 0; i < inst.nnodes; i++) {
        int from = i;
        int to = solution[i];
        double dx = inst.xcoord[from] - inst.xcoord[to];
        double dy = inst.ycoord[from] - inst.ycoord[to];
        cost += sqrt(dx * dx + dy * dy);
    }
    char instance_label[128];
    if (random_size > 0) {
        snprintf(instance_label, sizeof(instance_label), "random_%d", random_size);
    } else {
        snprintf(instance_label, sizeof(instance_label), "%s", tsp_file);
    }
    log_results_to_csv(log_file, instance_label, method, cost, elapsed);

    if (print_flag) {
        export_to_gnuplot(&inst, solution);
        system("gnuplot ./gnuplot_commands.txt");
    }
    free(solution);
    if (random_size > 0) {
        free(inst.xcoord);
        free(inst.ycoord);
    }
    return 0;
}
