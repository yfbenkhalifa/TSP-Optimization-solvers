//
// Created by WiZ on 18/01/2025.
//

#ifndef CPLEX_CALLBACKS_H
#define CPLEX_CALLBACKS_H

#include "common.h"
#include "tsp_common.h"

#endif //CPLEX_CALLBACKS_H

int CPXPUBLIC callback_function_candidate(CPXCALLBACKCONTEXTptr context, void *userhandle);
int CPXPUBLIC callback_function_relaxation(CPXCALLBACKCONTEXTptr context, void *userhandle);
int CPXPUBLIC callback_driver(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle);
