#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cplex.h>
#include <time.h>
#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"
#include <sys/stat.h>
#include <dirent.h>

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

typedef int (*TspMethodHandler)(instance*, int*, int);

typedef struct {
    const char *name;
    TspMethodHandler handler;
    int extra_arg;
} MethodEntry;

// Wrappers for methods with different signatures
int branch_and_cut_wrapper(instance *inst, int *solution, int verbose) {
    // Default local_branching_k = 17
    return cplex_tsp_branch_and_cut(inst, solution, verbose, 10);
}
int callback_candidate_wrapper(instance *inst, int *solution, int verbose) {
    return cplex_tsp_callback(inst, solution, verbose, CPX_CALLBACKCONTEXT_CANDIDATE, CPX_INFBOUND);
}
int callback_hard_fixing_wrapper(instance *inst, int *solution, int verbose) {
    return cplex_tsp_callback(inst, solution, verbose, CPX_CALLBACKCONTEXT_CANDIDATE, CPX_INFBOUND);
}
int not_implemented_wrapper(instance *inst, int *solution, int verbose) {
    log_message(LOG_LEVEL_WARNING, "Selected method is not implemented yet.");
    return -2;
}

int main(int argc, char *argv[]) {
    const char *method = NULL;
    const char *tsp_file = NULL;
    int print_flag = 0;
    int random_size = 0;
    int random_min_size = 0;
    int random_max_size = 0;
    int random_count = 1; // Default: 1 random instance if --random is used
    const char *log_file = "logfile.log";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = argv[++i];
        } else if (strcmp(argv[i], "--print") == 0) {
            print_flag = 1;
        } else if (strcmp(argv[i], "--random") == 0 && i + 1 < argc) {
            random_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--random-range") == 0 && i + 2 < argc) {
            random_min_size = atoi(argv[++i]);
            random_max_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--random-count") == 0 && i + 1 < argc) {
            random_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log_file = argv[++i];
        } else {
            tsp_file = argv[i];
        }
    }

    if (method == NULL || (tsp_file == NULL && random_size == 0 && random_min_size == 0)) {
        log_message(LOG_LEVEL_ERROR, "Usage: %s -m <method> [--print] [--random <size>] [--random-range <min> <max>] [--random-count <count>] [--log <logfile>] <tsp_file>", argv[0]);
        return 1;
    }

    int VERBOSE = 10000;
    instance inst;
    memset(&inst, 0, sizeof(instance));

    // Instance initialization
    struct stat path_stat;
    int is_directory = 0;
    if (tsp_file != NULL) {
        if (stat(tsp_file, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
            is_directory = 1;
        }
    }

    if (is_directory) {
        DIR *dir = opendir(tsp_file);
        if (!dir) {
            log_message(LOG_LEVEL_ERROR, "Failed to open directory: %s", tsp_file);
            return 1;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, ".tsp") != NULL) {
                char tsp_path[1024];
                snprintf(tsp_path, sizeof(tsp_path), "%s/%s", tsp_file, entry->d_name);
                strcpy(inst.input_file, tsp_path);
                read_input(&inst);
                if (inst.nnodes <= 0 || !inst.xcoord || !inst.ycoord) {
                    log_message(LOG_LEVEL_ERROR, "Failed to read TSP instance from file: %s", tsp_path);
                    continue;
                }
                int *solution = (int *) calloc(inst.nnodes, sizeof(int));
                if (!solution) {
                    log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for solution array.");
                    continue;
                }
                clock_t start = clock();
                int method_found = 0;
                int method_result = 0;
                MethodEntry methods[] = {
                    {"branch_and_cut", branch_and_cut_wrapper, 0},
                    {"cplex_callback_candidate", callback_candidate_wrapper, 0},
                    {"cplex_callback_hard_fixing", callback_hard_fixing_wrapper, 0},
                    {"tsp_greedy", not_implemented_wrapper, 0},
                };
                const int num_methods = sizeof(methods) / sizeof(MethodEntry);
                for (int i = 0; i < num_methods; ++i) {
                    if (strcmp(method, methods[i].name) == 0) {
                        method_found = 1;
                        method_result = methods[i].handler(&inst, solution, VERBOSE);
                        if (method_result != 0) {
                            log_message(LOG_LEVEL_ERROR, "Error in method '%s' (code %d)", methods[i].name, method_result);
                        }
                        break;
                    }
                }
                if (!method_found) {
                    log_message(LOG_LEVEL_ERROR, "Unknown method: %s", method);
                    free(solution);
                    continue;
                }
                clock_t end = clock();
                double elapsed = (double) (end - start) / CLOCKS_PER_SEC;

                // Check for invalid solution (e.g., all -1)
                int valid_solution = 1;
                for (int i = 0; i < inst.nnodes; i++) {
                    if (solution[i] < 0 || solution[i] >= inst.nnodes) {
                        valid_solution = 0;
                        break;
                    }
                }

                double cost = 0.0;
                if (valid_solution) {
                    for (int i = 0; i < inst.nnodes; i++) {
                        int from = i;
                        int to = solution[i];
                        double dx = inst.xcoord[from] - inst.xcoord[to];
                        double dy = inst.ycoord[from] - inst.ycoord[to];
                        cost += sqrt(dx * dx + dy * dy);
                    }
                } else {
                    log_message(LOG_LEVEL_WARNING, "No valid solution found for instance %s with method %s.", inst.input_file, method);
                    cost = -1.0;
                }

                char instance_label[128];
                // Use the real file name (without path) for logging
                const char *slash = strrchr(inst.input_file, '/');
                const char *bslash = strrchr(inst.input_file, '\\');
                const char *filename = inst.input_file;
                if (slash && bslash)
                    filename = (slash > bslash) ? slash + 1 : bslash + 1;
                else if (slash)
                    filename = slash + 1;
                else if (bslash)
                    filename = bslash + 1;
                snprintf(instance_label, sizeof(instance_label), "%s", filename);
                log_results_to_csv(log_file, instance_label, method, cost, elapsed);

                if (print_flag && valid_solution) {
                    export_to_gnuplot(&inst, solution);
                    system("gnuplot ./gnuplot_commands.txt");
                }

                free(solution);
                free(inst.xcoord);
                free(inst.ycoord);
            }
        }
        closedir(dir);
        return 0;
    } else {
        // Single file or random instance logic
        if (random_size > 0 || (random_min_size > 0 && random_max_size > 0)) {
            for (int rc = 0; rc < random_count; ++rc) {
                int curr_size = random_size;
                if (random_min_size > 0 && random_max_size > 0) {
                    curr_size = random_min_size + rand() % (random_max_size - random_min_size + 1);
                }
                inst.nnodes = curr_size;
                inst.xcoord = (double *) malloc(sizeof(double) * curr_size);
                inst.ycoord = (double *) malloc(sizeof(double) * curr_size);
                if (!inst.xcoord || !inst.ycoord) {
                    log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for random instance coordinates.");
                    return 1;
                }
                srand((unsigned int) (time(NULL) + rc));
                for (int i = 0; i < curr_size; i++) {
                    inst.xcoord[i] = ((double) rand() / RAND_MAX) * 1000.0;
                    inst.ycoord[i] = ((double) rand() / RAND_MAX) * 1000.0;
                }
                snprintf(inst.input_file, sizeof(inst.input_file), "random_%d_%d", curr_size, rc+1);
                int *solution = (int *) calloc(inst.nnodes, sizeof(int));
                if (!solution) {
                    log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for solution array.");
                    free(inst.xcoord);
                    free(inst.ycoord);
                    return 1;
                }
                clock_t start = clock();
                int method_found = 0;
                int method_result = 0;
                MethodEntry methods[] = {
                    {"branch_and_cut", branch_and_cut_wrapper, 0},
                    {"cplex_callback_candidate", callback_candidate_wrapper, 0},
                    {"cplex_callback_hard_fixing", callback_hard_fixing_wrapper, 0},
                    {"tsp_greedy", not_implemented_wrapper, 0},
                };
                const int num_methods = sizeof(methods) / sizeof(MethodEntry);
                for (int i = 0; i < num_methods; ++i) {
                    if (strcmp(method, methods[i].name) == 0) {
                        method_found = 1;
                        method_result = methods[i].handler(&inst, solution, VERBOSE);
                        if (method_result != 0) {
                            log_message(LOG_LEVEL_ERROR, "Error in method '%s' (code %d)", methods[i].name, method_result);
                        }
                        break;
                    }
                }
                if (!method_found) {
                    log_message(LOG_LEVEL_ERROR, "Unknown method: %s", method);
                    free(solution);
                    free(inst.xcoord);
                    free(inst.ycoord);
                    return 1;
                }
                clock_t end = clock();
                double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
                int valid_solution = 1;
                for (int i = 0; i < inst.nnodes; i++) {
                    if (solution[i] < 0 || solution[i] >= inst.nnodes) {
                        valid_solution = 0;
                        break;
                    }
                }
                double cost = 0.0;
                if (valid_solution) {
                    for (int i = 0; i < inst.nnodes; i++) {
                        int from = i;
                        int to = solution[i];
                        double dx = inst.xcoord[from] - inst.xcoord[to];
                        double dy = inst.ycoord[from] - inst.ycoord[to];
                        cost += sqrt(dx * dx + dy * dy);
                    }
                } else {
                    log_message(LOG_LEVEL_WARNING, "No valid solution found for instance %s with method %s.", inst.input_file, method);
                    cost = -1.0;
                }
                char instance_label[128];
                // Use the real file name (without path) for logging
                const char *slash = strrchr(inst.input_file, '/');
                const char *bslash = strrchr(inst.input_file, '\\');
                const char *filename = inst.input_file;
                if (slash && bslash)
                    filename = (slash > bslash) ? slash + 1 : bslash + 1;
                else if (slash)
                    filename = slash + 1;
                else if (bslash)
                    filename = bslash + 1;
                snprintf(instance_label, sizeof(instance_label), "%s", filename);
                log_results_to_csv(log_file, instance_label, method, cost, elapsed);
                if (print_flag && valid_solution) {
                    export_to_gnuplot(&inst, solution);
                    system("gnuplot ./gnuplot_commands.txt");
                }
                free(solution);
                free(inst.xcoord);
                free(inst.ycoord);
            }
            return 0;
        } else {
            strcpy(inst.input_file, tsp_file);
            read_input(&inst);
            if (inst.nnodes <= 0 || !inst.xcoord || !inst.ycoord) {
                log_message(LOG_LEVEL_ERROR, "Failed to read TSP instance from file: %s", tsp_file);
                return 1;
            }
        }

        int *solution = (int *) calloc(inst.nnodes, sizeof(int));
        if (!solution) {
            log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for solution array.");
            if (random_size > 0) {
                free(inst.xcoord);
                free(inst.ycoord);
            }
            return 1;
        }
        clock_t start = clock();
        int method_found = 0;
        int method_result = 0;
        MethodEntry methods[] = {
            {"branch_and_cut", branch_and_cut_wrapper, 0},
            {"cplex_callback_candidate", callback_candidate_wrapper, 0},
            {"cplex_callback_hard_fixing", callback_hard_fixing_wrapper, 0},
            {"tsp_greedy", not_implemented_wrapper, 0},
        };

        const int num_methods = sizeof(methods) / sizeof(MethodEntry);
        for (int i = 0; i < num_methods; ++i) {
            if (strcmp(method, methods[i].name) == 0) {
                method_found = 1;
                method_result = methods[i].handler(&inst, solution, VERBOSE);
                if (method_result != 0) {
                    log_message(LOG_LEVEL_ERROR, "Error in method '%s' (code %d)", methods[i].name, method_result);
                }
                break;
            }
        }
        if (!method_found) {
            log_message(LOG_LEVEL_ERROR, "Unknown method: %s", method);
            free(solution);
            if (random_size > 0) {
                free(inst.xcoord);
                free(inst.ycoord);
            }
            return 1;
        }
        clock_t end = clock();
        double elapsed = (double) (end - start) / CLOCKS_PER_SEC;

        // Check for invalid solution (e.g., all -1)
        int valid_solution = 1;
        for (int i = 0; i < inst.nnodes; i++) {
            if (solution[i] < 0 || solution[i] >= inst.nnodes) {
                valid_solution = 0;
                break;
            }
        }

        double cost = 0.0;
        if (valid_solution) {
            for (int i = 0; i < inst.nnodes; i++) {
                int from = i;
                int to = solution[i];
                double dx = inst.xcoord[from] - inst.xcoord[to];
                double dy = inst.ycoord[from] - inst.ycoord[to];
                cost += sqrt(dx * dx + dy * dy);
            }
        } else {
            log_message(LOG_LEVEL_WARNING, "No valid solution found for instance %s with method %s.", inst.input_file, method);
            cost = -1.0;
        }

        char instance_label[128];
        // Use the real file name (without path) for logging
        const char *slash = strrchr(inst.input_file, '/');
        const char *bslash = strrchr(inst.input_file, '\\');
        const char *filename = inst.input_file;
        if (slash && bslash)
            filename = (slash > bslash) ? slash + 1 : bslash + 1;
        else if (slash)
            filename = slash + 1;
        else if (bslash)
            filename = bslash + 1;
        snprintf(instance_label, sizeof(instance_label), "%s", filename);
        log_results_to_csv(log_file, instance_label, method, cost, elapsed);

        if (print_flag && valid_solution) {
            export_to_gnuplot(&inst, solution);
            system("gnuplot ./gnuplot_commands.txt");
        }

        free(solution);
        if (random_size > 0) {
            free(inst.xcoord);
            free(inst.ycoord);
        }
        return (valid_solution && method_result == 0) ? 0 : 1;
    }
}
