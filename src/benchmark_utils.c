#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Appends benchmark results with timing to a CSV file.
// methods: array of method names
// costs: array of costs (double)
// times: array of times (double, in seconds)
// num_methods: number of methods
void append_benchmark_result_with_time(const char* filename, const char* instance_name, const char** methods,
                                       const double* costs, const double* times, int num_methods) {
    FILE* csv_file = fopen(filename, "r");
    char** lines = NULL;
    int line_count = 0;
    size_t linecap = 0;
    ssize_t linelen;
    char* line = NULL;
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

    // Prepare header
    char header[4096] = {0};
    if (line_count > 0) {
        strncpy(header, lines[0], sizeof(header) - 1);
    } else {
        strcat(header, "Instance");
        for (i = 0; i < num_methods; i++) {
            strcat(header, ",");
            strcat(header, methods[i]);
            strcat(header, "_cost");
            strcat(header, ",");
            strcat(header, methods[i]);
            strcat(header, "_time");
        }
        strcat(header, "\n");
        lines = realloc(lines, sizeof(char*) * (line_count + 1));
        lines[0] = strdup(header);
        line_count = 1;
    }

    // Always append a new row for the instance
    char new_row[4096] = {0};
    strcat(new_row, instance_name);
    for (j = 0; j < num_methods; j++) {
        char cost_str[64], time_str[64];
        snprintf(cost_str, sizeof(cost_str), ",%.2f", costs[j]);
        snprintf(time_str, sizeof(time_str), ",%.6f", times[j]);
        strcat(new_row, cost_str);
        strcat(new_row, time_str);
    }
    strcat(new_row, "\n");
    lines = realloc(lines, sizeof(char*) * (line_count + 1));
    lines[line_count] = strdup(new_row);
    line_count++;

    // Write all lines back to file
    csv_file = fopen(filename, "w");
    for (i = 0; i < line_count; i++) {
        fputs(lines[i], csv_file);
        free(lines[i]);
    }
    free(lines);
    fclose(csv_file);
}

