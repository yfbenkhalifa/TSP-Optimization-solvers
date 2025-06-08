//
// Created by WiZ on 13/03/2025.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "tsp_common.h"
#include "cplex_builder.h"

void append_benchmark_result(const char* filename, const char* instance_name, const char** methods,
                           const double* costs, int num_methods) {
    // Read the existing CSV into memory
    FILE* csv_file = fopen(filename, "r");
    char** lines = NULL;
    int line_count = 0;
    size_t linecap = 0;
    ssize_t linelen;
    char* line = NULL;
    int header_found = 0;
    int method_indices[32] = {0}; // Support up to 32 methods
    int method_exists[32] = {0};
    int method_count = 0;
    int instance_row = -1;
    int i, j;

    // Read all lines
    if (csv_file) {
        while ((linelen = getline(&line, &linecap, csv_file)) > 0) {
            lines = realloc(lines, sizeof(char*) * (line_count + 1));
            lines[line_count] = strdup(line);
            line_count++;
        }
        fclose(csv_file);
    }

    // Parse header or create new
    char header[2048] = {0};
    if (line_count > 0) {
        strncpy(header, lines[0], sizeof(header) - 1);
        header_found = 1;
    } else {
        // Create header
        char num_methods_str[16];
        sprintf(num_methods_str, "%d", num_methods);
        strcat(header, num_methods_str);
        for (i = 0; i < num_methods; i++) {
            strcat(header, ",");
            strcat(header, methods[i]);
        }
        strcat(header, "\n");
        lines = realloc(lines, sizeof(char*) * (line_count + 1));
        lines[0] = strdup(header);
        line_count = 1;
        header_found = 1;
    }

    // Parse header columns
    char* header_copy = strdup(header);
    char* token = strtok(header_copy, ",\n");
    char* columns[64];
    int ncols = 0;
    while (token) {
        columns[ncols++] = token;
        token = strtok(NULL, ",\n");
    }
    // Check which methods exist
    for (i = 1; i < ncols; i++) {
        for (j = 0; j < num_methods; j++) {
            if (strcmp(columns[i], methods[j]) == 0) {
                method_exists[j] = 1;
                method_indices[j] = i;
            }
        }
    }
    // Add new methods to header if needed
    int header_changed = 0;
    for (j = 0; j < num_methods; j++) {
        if (!method_exists[j]) {
            // Add to header
            size_t len = strlen(lines[0]);
            lines[0] = realloc(lines[0], len + strlen(methods[j]) + 2);
            strcat(lines[0], ",");
            strcat(lines[0], methods[j]);
            method_indices[j] = ncols++;
            header_changed = 1;
        }
    }
    free(header_copy);

    // Find instance row
    for (i = 1; i < line_count; i++) {
        char* row_copy = strdup(lines[i]);
        char* row_token = strtok(row_copy, ",\n");
        if (row_token && strcmp(row_token, instance_name) == 0) {
            instance_row = i;
            free(row_copy);
            break;
        }
        free(row_copy);
    }

    // If instance not found, add new row
    if (instance_row == -1) {
        char new_row[2048] = {0};
        strcat(new_row, instance_name);
        for (i = 1; i < ncols; i++) {
            strcat(new_row, ",");
            strcat(new_row, "");
        }
        lines = realloc(lines, sizeof(char*) * (line_count + 1));
        lines[line_count] = strdup(new_row);
        instance_row = line_count;
        line_count++;
    }

    // Update the cell(s) for this instance and method(s)
    char* row_buf = strdup(lines[instance_row]);
    char* row_fields[64];
    int row_field_count = 0;
    char* row_token = strtok(row_buf, ",\n");
    while (row_token) {
        row_fields[row_field_count++] = row_token;
        row_token = strtok(NULL, ",\n");
    }
    // Expand row if needed
    if (row_field_count < ncols) {
        for (i = row_field_count; i < ncols; i++) {
            row_fields[i] = "";
        }
        row_field_count = ncols;
    }
    // Set the cost for each method
    for (j = 0; j < num_methods; j++) {
        char cost_str[32];
        snprintf(cost_str, sizeof(cost_str), "%.2f", costs[j]);
        row_fields[method_indices[j]] = strdup(cost_str);
    }
    // Rebuild the row
    char new_row[2048] = {0};
    strcat(new_row, row_fields[0]);
    for (i = 1; i < row_field_count; i++) {
        strcat(new_row, ",");
        strcat(new_row, row_fields[i]);
    }
    strcat(new_row, "\n");
    free(lines[instance_row]);
    lines[instance_row] = strdup(new_row);
    free(row_buf);

    // Write all lines back to file
    csv_file = fopen(filename, "w");
    for (i = 0; i < line_count; i++) {
        fputs(lines[i], csv_file);
        free(lines[i]);
    }
    free(lines);
    fclose(csv_file);
}

void generate_test_bed(instance** test_bed, int size, int seed, int min_nodes, int max_nodes) {
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
        int nodes = min_nodes + (rand() % max_nodes); // Random size between 10 and 100 nodes

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
    const char* methods[] = {
        "Tabu Search with EM for starting solution (10 Neighborhood Size)",
        "Tabu Search with EM for starting solution (20 Neighborhood Size)",
        "Tabu Search with EM for starting solution (30 Neighborhood Size)"

    };
    double* costs = ARRAY_ALLOC(sizeof(double), num_methods);
    if (!costs) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for costs");
        return 1;
    }

    // Output CSV filename (default)
    char csv_filename[256] = "../benchmark.csv";
    if (argc > 1) {
        strncpy(csv_filename, argv[1], sizeof(csv_filename) - 1);
        csv_filename[sizeof(csv_filename) - 1] = '\0';
    }

    // Reduce number of instances for faster benchmarking
    int num_instances = 100;
    int seed = 42;
    srand(seed); // Set the random seed once

    instance* test_bed = NULL;
    generate_test_bed(&test_bed, num_instances, seed, 50, 150);
    if (!test_bed) {
        free(costs);
        return 1;
    }

    for (int i = 0; i < num_instances; i++) {
        instance* inst = &test_bed[i];
        char instance_name[32];
        snprintf(instance_name, sizeof(instance_name), "random_%d", inst->nnodes);

        // Allocate solution structures once and reuse
        Solution solution;
        solution.solution = (int*)malloc(inst->nnodes * sizeof(int));
        if (!solution.solution) {
            log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for solution");
            continue;
        }

        int* starting_solution = (int*)malloc(inst->nnodes * sizeof(int));
        if (!starting_solution) {
            free(solution.solution);
            log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for starting solution");
            continue;
        }

        tsp_extra_mileage(inst, &solution, euclidean_most_distant_pair(inst));
        memcpy(starting_solution, solution.solution, inst->nnodes * sizeof(int));

        // Method 1: GRASP with 2OPT with fixed start
        tabu_search(inst, starting_solution, &solution, 10);
        costs[0] = compute_solution_cost(inst, solution.solution);

        // Method 2: GRASP with 2OPT with random start
        tabu_search(inst, starting_solution, &solution, 20);
        costs[1] = compute_solution_cost(inst, solution.solution);

        // Method 3: Tabu search
        tabu_search(inst, starting_solution, &solution, 30);
        costs[2] = compute_solution_cost(inst, solution.solution);

        // Write results using the new append_benchmark_result
        append_benchmark_result(csv_filename, instance_name, methods, costs, num_methods);

        free(starting_solution);
        free(solution.solution);
    }

    log_message(LOG_LEVEL_INFO, "Benchmark completed successfully");
    log_message(LOG_LEVEL_INFO, "Cleaning up...");

    cleanup:
    // Free allocated memory
    for (int i = 0; i < num_instances; i++) {
        free(test_bed[i].xcoord);
        free(test_bed[i].ycoord);
        free(test_bed[i].demand);
        free(test_bed[i].solution);
    }
    free(test_bed);
    free(costs);

    return 0;
}
