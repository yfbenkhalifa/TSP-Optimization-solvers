//
// Created by wiz on 3/19/24.
//

#include "utils.h"
#define VERBOSE 50

instance generate_instance(int nnodes, double *demand, double *xcoord, double *ycoord, int depot, double capacity, int nveh) {
    instance inst;
    inst.nnodes = nnodes;
    inst.demand = (double *) malloc(nnodes * sizeof(double));
    inst.xcoord = (double *) malloc(nnodes * sizeof(double));
    inst.ycoord = (double *) malloc(nnodes * sizeof(double));
    inst.solution = (int *) malloc(nnodes * sizeof(int));

    for (int i = 0; i < nnodes; i++) {
        inst.demand[i] = demand[i];
        inst.xcoord[i] = xcoord[i];
        inst.ycoord[i] = ycoord[i];
        inst.solution[i] = -1; // Initialize solution to -1
    }

    inst.depot = depot;
    inst.capacity = capacity;
    inst.nveh = nveh;

    // Initialize other fields to default values
    inst.best_cost_value = 0;
    inst.best_known_solution_value = 0;
    inst.model_type = 0;
    inst.old_benders = 0;
    inst.randomseed = 0;
    inst.num_threads = 1;
    inst.timelimit = 0;
    strcpy(inst.input_file, "generated_instance");
    strcpy(inst.node_file, "");
    inst.available_memory = 0;
    inst.max_nodes = -1;
    inst.cutoff = 0;
    inst.integer_costs = 0;
    inst.tstart = 0;
    inst.zbest = 0;
    inst.tbest = 0;
    inst.best_sol = NULL;
    inst.best_lb = 0;
    inst.load_min = NULL;
    inst.load_max = NULL;
    inst.xstart = 0;
    inst.qstart = 0;
    inst.bigqstart = 0;
    inst.sstart = 0;
    inst.bigsstart = 0;
    inst.ystart = 0;
    inst.fstart = 0;
    inst.zstart = 0;

    return inst;
}

bool compare_solutions(const int *solution1, const int *solution2, int size) {
    for (int i = 0; i < size; i++) {
        if (solution1[i] != solution2[i]) {
            return false;
        }
    }
    return true;
}

double euclidean_distance(double x1, double y1, double x2, double y2, bool round){
    double dx = x1 - x2;
    double dy = y1 - y2;
    if (round) return (int) (sqrt(dx*dx + dy*dy) + 0.5);
    return sqrt(dx*dx + dy*dy);
}


void read_input(instance *inst)
{

    FILE *fin = fopen(inst->input_file, "r");
    if ( fin == NULL ) print_error(" input file not found!");

    inst->nnodes = -1;
    inst->depot = -1;
    inst->nveh = -1;

    char line[180];
    char *par_name;
    char *token1;
    char *token2;

    int active_section = 0; // =1 NODE_COORD_SECTION, =2 DEMAND_SECTION, =3 DEPOT_SECTION

    int do_print = ( VERBOSE >= 1000 );

    while ( fgets(line, sizeof(line), fin) != NULL )
    {
        if ( VERBOSE >= 2000 ) { printf("%s",line); fflush(NULL); }
        if ( strlen(line) <= 1 ) continue; // skip empty lines
        par_name = strtok(line, " :");
        if ( VERBOSE >= 3000 ) { printf("parameter \"%s\" ",par_name); fflush(NULL); }

        if ( strncmp(par_name, "NAME", 4) == 0 )
        {
            active_section = 0;
            continue;
        }

        if ( strncmp(par_name, "COMMENT", 7) == 0 )
        {
            active_section = 0;
            token1 = strtok(NULL, "");
            if ( VERBOSE >= 10 ) printf(" ... solving instance %s with model %d\n\n", token1, inst->model_type);
            continue;
        }

        if ( strncmp(par_name, "TYPE", 4) == 0 )
        {
            token1 = strtok(NULL, " :");
            if ( strncmp(token1, "CVRP",4) != 0 ) print_error(" format error:  only TYPE == CVRP implemented so far!!!!!!");
            active_section = 0;
            continue;
        }


        if ( strncmp(par_name, "DIMENSION", 9) == 0 )
        {
            if ( inst->nnodes >= 0 ) print_error(" repeated DIMENSION section in input file");
            token1 = strtok(NULL, " :");
            inst->nnodes = atoi(token1);
            if ( do_print ) printf(" ... nnodes %d\n", inst->nnodes);
            inst->demand = (double *) calloc(inst->nnodes, sizeof(double));
            inst->xcoord = (double *) calloc(inst->nnodes, sizeof(double));
            inst->ycoord = (double *) calloc(inst->nnodes, sizeof(double));
            active_section = 0;
            continue;
        }

        if ( strncmp(par_name, "CAPACITY", 8) == 0 )
        {
            token1 = strtok(NULL, " :");
            inst->capacity = atof(token1);
            if ( do_print ) printf(" ... vehicle capacity %lf\n", inst->capacity);
            active_section = 0;
            continue;
        }


        if ( strncmp(par_name, "VEHICLES", 8) == 0 )
        {
            token1 = strtok(NULL, " :");
            inst->nveh = atoi(token1);
            if ( do_print ) printf(" ... n. vehicles %d\n", inst->nveh);
            active_section = 0;
            continue;
        }


        if ( strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0 )
        {
            token1 = strtok(NULL, " :");
            if ( strncmp(token1, "EUC_2D", 6) != 0 ) print_error(" format error:  only EDGE_WEIGHT_TYPE == EUC_2D implemented so far!!!!!!");
            active_section = 0;
            continue;
        }

        if ( strncmp(par_name, "NODE_COORD_SECTION", 18) == 0 )
        {
            if ( inst->nnodes <= 0 ) print_error(" ... DIMENSION section should appear before NODE_COORD_SECTION section");
            active_section = 1;
            continue;
        }

        if ( strncmp(par_name, "DEMAND_SECTION", 14) == 0 )
        {
            if ( inst->nnodes <= 0 ) print_error(" ... DIMENSION section should appear before DEMAND_SECTION section");
            active_section = 2;
            continue;
        }

        if ( strncmp(par_name, "DEPOT_SECTION", 13) == 0 )
        {
            if ( inst->depot >= 0 ) print_error(" ... DEPOT_SECTION repeated??");
            active_section = 3;
            continue;
        }


        if ( strncmp(par_name, "EOF", 3) == 0 )
        {
            active_section = 0;
            break;
        }

        if (strncmp(par_name, "BEST_KNOWN", 10) == 0)
        {
            token1 = strtok(NULL, " :");
            inst->best_known_solution_value = atof(token1);
            if (do_print) printf(" ... best known solution value %lf\n", inst->best_known_solution_value);
            active_section = 0;
            continue;
        }


        if ( active_section == 1 ) // within NODE_COORD_SECTION
        {
            int i = atoi(par_name) - 1;
            if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");
            token1 = strtok(NULL, " :,");
            token2 = strtok(NULL, " :,");
            inst->xcoord[i] = atof(token1);
            inst->ycoord[i] = atof(token2);
            if ( do_print ) printf(" ... node %4d at coordinates ( %15.7lf , %15.7lf )\n", i+1, inst->xcoord[i], inst->ycoord[i]);
            continue;
        }

        if ( active_section == 2 ) // within DEMAND_SECTION
        {
            int i = atoi(par_name) - 1;
            if ( i < 0 || i >= inst->nnodes ) print_error(" ... unknown node in NODE_COORD_SECTION section");
            token1 = strtok(NULL, " :,");
            inst->demand[i] = atof(token1);
            if ( do_print ) printf(" ... node %4d has demand %10.5lf\n", i+1, inst->demand[i]);
            continue;
        }

        if ( active_section == 3 ) // within DEPOT_SECTION
        {
            int i = atoi(par_name) - 1;
            if ( i < 0 || i >= inst->nnodes ) continue;
            if ( inst->depot >= 0 ) print_error(" ... multiple depots not supported in DEPOT_SECTION");
            inst->depot = i;
            if ( do_print ) printf(" ... depot node %d\n", inst->depot+1);
            continue;
        }

        printf(" final active section %d\n", active_section);
        print_error(" ... wrong format for the current simplified parser!!!!!!!!!");

    }

    fclose(fin);

}

bool has_duplicates(instance* inst, const int* solution){
    if (inst->nnodes == -1) return false;
    int *hash_set = (int *)calloc(inst->nnodes, sizeof(int));
    if (hash_set == NULL) {
        return false; // Memory allocation failed
    }

    for (int i = 0; i < inst->nnodes; i++) {
        int value = solution[i];
        if (hash_set[value] == 1) {
            free(hash_set);
            return true; // Duplicate found
        }
        hash_set[value] = 1;
    }

    free(hash_set);
    return false;
}

bool is_acyclic(instance *inst, const int *solution) {
    if (inst->nnodes == -1) return false;

    for (int i = 0; i < inst->nnodes; i++) {
        if(solution[i] == i) return false;
    }

    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {
            if (i == solution[j] && j == solution[i]) {
                return false;
            }
        }
    }
    return true;
}






