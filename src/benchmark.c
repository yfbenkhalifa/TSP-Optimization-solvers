//
// Created by WiZ on 13/03/2025.
//

#include <stdio.h>
#include "utils.h"
#include "tsp_common.h"

void append_benchmark_result(const char* filename, const char* instance_name, const char** methods,
                           const double* costs, int num_methods) {
    FILE* csv_file = fopen(filename, "a"); // Open in append mode
    if (csv_file == NULL) {
        log_message(LOG_LEVEL_ERROR, "Failed to open CSV file %s", filename);
        return;
    }

    // Check if file is empty to write header
    fseek(csv_file, 0, SEEK_END);
    if (ftell(csv_file) == 0) {
        fprintf(csv_file, "%d" ,num_methods);
        for (int i = 0; i < num_methods; i++) {
            fprintf(csv_file, ",%s", methods[i]);
        }
        fprintf(csv_file, "\n");
    }

    // Write instance results
    fprintf(csv_file, "%s", instance_name);
    for (int i = 0; i < num_methods; i++) {
        fprintf(csv_file, ",%.2f", costs[i]);
    }
    fprintf(csv_file, "\n");

    fclose(csv_file);
}



void generate_test_bed(instance** test_bed, int size, int seed) {
    // Allocate array of instances
    *test_bed = (instance*)malloc(size * sizeof(instance));
    if (*test_bed == NULL) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for test bed");
        return;
    }

    // Initialize each instance
    for (int i = 0; i < size; i++) {
        // Generate random instance with different seeds
        int instance_seed = seed + i;
        int nodes = 200 + (rand() % 1000); // Random size between 10 and 100 nodes

        // Initialize solution array
        (*test_bed)[i].solution = (int*)malloc(nodes * sizeof(int));
        if ((*test_bed)[i].solution == NULL) {
            log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for instance %d solution", i);
            // Clean up previously allocated memory
            for (int j = 0; j < i; j++) {
                free((*test_bed)[j].xcoord);
                free((*test_bed)[j].ycoord);
                free((*test_bed)[j].demand);
                free((*test_bed)[j].solution);
            }
            free(*test_bed);
            return;
        }

        // Generate the random instance
        generate_random_instance(&(*test_bed)[i], nodes, instance_seed);
    }
}



int main(int argc, char *argv[]) {
    int num_methods = 3;
    const char* methods[] = {"GRASP", "2OPT", "ExtraMileage"};
    double* costs = ARRAY_ALLOC(sizeof(double), num_methods);


    instance* test_bed = NULL;
    int num_instances = 100;
    int seed = 42;

    generate_test_bed(&test_bed, num_instances, seed);

    for (int i = 0; i < num_instances; i++) {
        instance inst = test_bed[i];
        char instance_name[32];
        snprintf(instance_name, sizeof(instance_name), "random_%d", inst.nnodes);

        int* solution = (int*)malloc(inst.nnodes * sizeof(int));
        tsp_grasp(&inst, 0);
        costs[0] = compute_solution_cost(&inst, inst.solution);
        tsp_two_opt(&inst);
        costs[1] = compute_solution_cost(&inst, inst.solution);
        tsp_extra_mileage(&inst, euclidean_most_distant_pair(&inst));
        costs[2] = compute_solution_cost(&inst, inst.solution);
        append_benchmark_result("../benchmark.csv", instance_name, methods, costs, num_methods);
        free(solution);
    }

    for (int i = 0; i < num_instances; i++) {
        free(test_bed[i].xcoord);
        free(test_bed[i].ycoord);
        free(test_bed[i].demand);
        free(test_bed[i].solution);
    }
    free(test_bed);

    return 0;
}
