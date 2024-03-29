//
// Created by wiz on 3/19/24.
//

#ifndef OR2PROJECT_UTILS_H
#define OR2PROJECT_UTILS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cplex.h>
#include <pthread.h>

#include "tsp.h"
#endif //OR2PROJECT_UTILS_H

double euclidean_distance(double x1, double y1, double x2, double y2, bool round);
void read_input(instance *inst);
void parse_command_line(int argc, char** argv, instance *inst);
void free_instance(instance *inst);
void print_error(const char *err);

