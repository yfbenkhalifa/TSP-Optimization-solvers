#ifndef BENCHMARK_UTILS_H
#define BENCHMARK_UTILS_H

#include <stdio.h>

// Declare functions implemented in benchmark_utils.c
// Add your function prototypes here, for example:
// void some_function(args...);
void append_benchmark_result_with_time(const char* filename, const char* instance_name, const char** methods,
                                       const double* costs, const double* times, int num_methods);

#endif // BENCHMARK_UTILS_H

