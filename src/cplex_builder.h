// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include <cplex.h>
#include "tsp_common.h"


void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif // CPLEX_BUILDER_H