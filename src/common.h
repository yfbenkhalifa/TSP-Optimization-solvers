//
// Created by WiZ on 20/11/2024.
//

#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define FATAL_WRAP(call) \
do { \
if (!(call)) { \
fprintf(stderr, "Fatal error: %s failed at %s:%d\n", #call, __FILE__, __LINE__); \
exit(EXIT_FAILURE); \
} \
} while (0)

#define ERROR_WRAP(call) \
do { \
if (!(call)) { \
fprintf(stderr, "Error: %s failed at %s:%d\n", #call, __FILE__, __LINE__); \
} \
} while (0)


void* ARRAY_ALLOC(size_t num, size_t size);

#endif //COMMON_H
