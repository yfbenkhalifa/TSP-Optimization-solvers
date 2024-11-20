#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include <string.h>

#include "tsp.h"
#include "utils.h"
#include "cplex_builder.h"
int main() {
    instance inst;
    strcpy(inst.input_file, "../data/tsp_mock.tsp");
    read_input(&inst);
}
