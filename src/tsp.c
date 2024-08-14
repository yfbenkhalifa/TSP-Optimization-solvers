//
// Created by wiz on 3/29/24.
//

#include "tsp.h"

#define VERBOSE 50

void two_opt_swap(instance* instance, pair p1, pair p2)
{
    // Assume: (p1.node1 -> p1.node2) and (p2.node1 -> p2.node2) is directed as such
    int a1 = p1.node1;
    int a2 = p1.node2;
    int b1 = p2.node1;
    int b2 = p2.node2;

    instance->solution[a1] = b1;
    instance->solution[a2] = b2;
}

void undo_two_opt_swap(instance* instance, pair p1, pair p2)
{
    // Assume: (p1.node1 -> p1.node2) and (p2.node1 -> p2.node2) is directed as such
    int a1 = p1.node1;
    int a2 = p1.node2;
    int b1 = p2.node1;
    int b2 = p2.node2;

    instance->solution[a1] = b2;
    instance->solution[b1] = a2;
}


void reverse_path(instance* instance, int start_node, int end_node)
{
    int currentNode = start_node;

    int current_solution = instance->solution[currentNode];
    int next_node = instance->solution[current_solution];
    while (currentNode != end_node)
    {
        next_node = instance->solution[current_solution];
        instance->solution[current_solution] = currentNode;
        currentNode = current_solution;
        current_solution = next_node;
    }
}

void reverse_path2(instance* instance, int start_node, int end_node)
{
    int* path = (int*)malloc(instance->nnodes * sizeof(int));
    int path_length = 0;
    int current_node = start_node;

    // Collect the path from start_node to end_node
    while (current_node != end_node) {
        path[path_length++] = current_node;
        current_node = instance->solution[current_node];
    }
    path[path_length++] = end_node;

    // Reverse the path
    for (int i = 0; i < path_length / 2; ++i) {
        int temp = path[i];
        path[i] = path[path_length - 1 - i];
        path[path_length - 1 - i] = temp;
    }

    // Update the solution with the reversed path
    for (int i = 0; i < path_length - 1; ++i) {
        instance->solution[path[i]] = path[i + 1];
    }

    free(path);
}

double tsp_two_opt(instance* instance)
{
    double deltaCost = 0;
    int min_a1 = -1;
    int min_a2 = -1;
    int min_b1 = -1;
    int min_b2 = -1;
    // cycle through all pairs of edges
    for (int i = 0; i < instance->nnodes; i++)
    {
        for (int j = 0; j < instance->nnodes; j++)
        {
            if (i == j) continue;
            int a1 = i;
            int a2 = instance->solution[i];
            int b1 = j;
            int b2 = instance->solution[j];

            double cost_a1_a2 = euclidean_distance(instance->xcoord[a1],
                                              instance->ycoord[a1],
                                              instance->xcoord[a2],
                                              instance->ycoord[a2], false);

            double cost_b1_b2 = euclidean_distance(instance->xcoord[b1],
                                              instance->ycoord[b1],
                                              instance->xcoord[b2],
                                              instance->ycoord[b2], false);

            double cost_a1_b1 = euclidean_distance(instance->xcoord[a1],
                                                 instance->ycoord[a1],
                                                 instance->xcoord[b1],
                                                 instance->ycoord[b1], false);

            double cost_a2_b2 = euclidean_distance(instance->xcoord[a2],
                                                 instance->ycoord[a2],
                                                 instance->xcoord[b2],
                                                 instance->ycoord[b2], false);

            double currentDeltaCost = (cost_a1_b1 + cost_a2_b2) - (cost_a1_a2 + cost_b1_b2);
            if (currentDeltaCost < deltaCost)
            {
                deltaCost = currentDeltaCost;
                min_a1 = a1;
                min_a2 = a2;
                min_b1 = b1;
                min_b2 = b2;
            }
        }
    }

    if (deltaCost < 0)
    {
        int starting_reverse_node = instance->solution[min_a2];
        instance->solution[min_a1] = min_b1;
        instance->solution[min_a2] = min_b2;
        reverse_path(instance, starting_reverse_node, min_b1);

    }

    return deltaCost;
}

void tsp_greedy(instance* inst, int starting_node)
{
    int current_node_index = starting_node;
    int remaining_nodes_count = inst->nnodes;
    solution nearest_node;
    int* remaining_nodes = (int*)malloc(inst->nnodes * sizeof(int));


    inst->solution = (int*)malloc(inst->nnodes * sizeof(int));
    inst->best_cost_value = 0;
    // Initialize solution
    for (int i = 0; i < inst->nnodes; i++)
    {
        inst->solution[i] = -1;
        remaining_nodes[i] = i;
    }


    remaining_nodes[current_node_index] = remaining_nodes[--remaining_nodes_count];

    for (int i = 0; i < inst->nnodes; i++)
    {
        nearest_node = euclidean_nearest_node(inst,
                                              current_node_index,
                                              remaining_nodes,
                                              &remaining_nodes_count);

        if (nearest_node.node == -1)
        {
            inst->solution[current_node_index] = starting_node;
            inst->best_cost_value += euclidean_distance(inst->xcoord[current_node_index],
                                                        inst->ycoord[current_node_index],
                                                        inst->xcoord[starting_node],
                                                        inst->ycoord[starting_node], false);
            continue;
        }
        inst->solution[current_node_index] = nearest_node.node;
        inst->best_cost_value += nearest_node.cost;
        current_node_index = nearest_node.node;
    }
}

void tsp_extra_mileage(instance* inst, pair starting_pair)
{
    heuristic_state state;
    pair current_pair = starting_pair;

    initialize_instance(inst, &state);

    inst->solution[current_pair.node1] = current_pair.node2;
    inst->solution[current_pair.node2] = current_pair.node1;
    state.covered_nodes[state.covered_nodes_count++] = current_pair.node1;
    state.covered_nodes[state.covered_nodes_count++] = current_pair.node2;
    state.uncovered_nodes[current_pair.node1] = state.uncovered_nodes[--state.uncovered_nodes_count];
    state.uncovered_nodes[current_pair.node2] = state.uncovered_nodes[--state.uncovered_nodes_count];

    while (state.covered_nodes_count < inst->nnodes)
    {
        solution best_node;
        for (int i = 0; i < state.covered_nodes_count; i++)
        {
            int current_node = state.covered_nodes[i];
            int current_node_opposite = inst->solution[current_node];
            double min_distance_delta = INFINITY;
            best_node.node = -1;
            best_node.node_index = -1;
            for (int j = 0; j < state.uncovered_nodes_count; j++)
            {
                double distance1 = euclidean_distance(inst->xcoord[current_node],
                                                      inst->ycoord[current_node],
                                                      inst->xcoord[state.uncovered_nodes[j]],
                                                      inst->ycoord[state.uncovered_nodes[j]], false);
                double distance2 = euclidean_distance(inst->xcoord[state.uncovered_nodes[j]],
                                                      inst->ycoord[state.uncovered_nodes[j]],
                                                      inst->xcoord[current_node_opposite],
                                                      inst->ycoord[current_node_opposite], false);
                double existing_pair_distance = euclidean_distance(inst->xcoord[current_node],
                                                                   inst->ycoord[current_node],
                                                                   inst->xcoord[current_node_opposite],
                                                                   inst->ycoord[current_node_opposite], false);

                double distance_delta = distance1 + distance2 - existing_pair_distance;

                if (distance_delta < min_distance_delta)
                {
                    min_distance_delta = distance_delta;
                    best_node.node = state.uncovered_nodes[j];
                    best_node.node_index = j;
                    best_node.cost = distance_delta;
                }
            }
            if (best_node.node > -1)
            {
                inst->solution[current_node] = best_node.node;
                inst->solution[best_node.node] = current_node_opposite;
                state.covered_nodes[state.covered_nodes_count++] = best_node.node;
                state.uncovered_nodes[best_node.node_index] = state.uncovered_nodes[--state.uncovered_nodes_count];
            }
        }
    }
}


void initialize_instance(instance* inst, heuristic_state* state)
{
    state->covered_nodes_count = 0;
    state->uncovered_nodes_count = inst->nnodes;
    inst->solution = (int*)malloc(inst->nnodes * sizeof(int));
    state->covered_nodes = (int*)malloc(inst->nnodes * sizeof(int));
    state->uncovered_nodes = (int*)malloc(inst->nnodes * sizeof(int));
    for (int i = 0; i < inst->nnodes; ++i)
    {
        inst->solution[i] = -1;
        state->covered_nodes[i] = i;
        state->uncovered_nodes[i] = i;
    }
}


pair euclidean_most_distant_pair(instance* inst)
{
    double max_distance = 0;
    pair most_distant_pair;
    most_distant_pair.node1 = -1;
    most_distant_pair.node2 = -1;

    for (int i = 0; i < inst->nnodes; i++)
    {
        for (int j = 0; j < inst->nnodes; j++)
        {
            if (i == j) continue;
            double distance = euclidean_distance(inst->xcoord[i],
                                                 inst->ycoord[i],
                                                 inst->xcoord[j],
                                                 inst->ycoord[j],
                                                 false);
            if (distance > max_distance)
            {
                max_distance = distance;
                most_distant_pair.node1 = i;
                most_distant_pair.node2 = j;
            }
        }
    }
    return most_distant_pair;
}


solution euclidean_nearest_node(instance* instance, int node, int* remaining_nodes, int* remaining_nodes_count)
{
    double min_distance = INFINITY;
    solution nearest_node;
    nearest_node.node = -1;
    nearest_node.node_index = -1;
    nearest_node.cost = min_distance;


    for (int j = 0; j < *remaining_nodes_count; j++)
    {
        if (node == remaining_nodes[j]) continue;
        double distance = euclidean_distance(instance->xcoord[node], instance->ycoord[node],
                                             instance->xcoord[remaining_nodes[j]], instance->ycoord[remaining_nodes[j]],
                                             false);
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_node.node = remaining_nodes[j];
            nearest_node.node_index = j;
        }
    }
    nearest_node.cost = min_distance;
    remaining_nodes[nearest_node.node_index] = remaining_nodes[--*remaining_nodes_count];
    return nearest_node;
}

double compute_solution_cost(instance* inst, const int* solution)
{
    double cost = 0;
    for (int i = 0; i < inst->nnodes; i++)
    {
        if (solution[i] == -1) continue;
        cost += euclidean_distance(inst->xcoord[i],
                                   inst->ycoord[i],
                                   inst->xcoord[solution[i]],
                                   inst->ycoord[solution[i]], false);
    }
    return cost;
}

double compute_geometric_mean(instance testBed[], int testBedSolutions[])
{
    double product = 1.0;
    int num_instances = sizeof(testBed) / sizeof(instance);

    for (int i = 0; i < num_instances; i++)
    {
        double cost = testBedSolutions[i];
        product *= cost;
    }

    return pow(product, 1.0 / num_instances);
}

void tabu_search(int *initial_solution, int size) {
    int **tabu_list = (int **)malloc(TABU_TENURE * sizeof(int *));
    for (int i = 0; i < TABU_TENURE; i++) {
        tabu_list[i] = (int *)malloc(size * sizeof(int));
    }
    int tabu_index = 0;

    Solution best_solution;
    best_solution.solution = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        best_solution.solution[i] = initial_solution[i];
    }
    best_solution.cost = evaluate_solution(initial_solution, size);

    Solution current_solution = best_solution;

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        int num_neighbors = size; // Number of neighbors to generate
        int **neighbors = (int **)malloc(num_neighbors * sizeof(int *));
        for (int i = 0; i < num_neighbors; i++) {
            neighbors[i] = (int *)malloc(size * sizeof(int));
        }
        generate_neighbors(current_solution.solution, size, neighbors, num_neighbors);

        Solution best_neighbor;
        best_neighbor.solution = NULL;
        best_neighbor.cost = INT_MAX;

        for (int i = 0; i < num_neighbors; i++) {
            int cost = evaluate_solution(neighbors[i], size);
            if (cost < best_neighbor.cost && !is_tabu(neighbors[i], tabu_list, TABU_TENURE, size)) {
                best_neighbor.solution = neighbors[i];
                best_neighbor.cost = cost;
            }
        }

        if (best_neighbor.solution != NULL) {
            current_solution = best_neighbor;
            if (current_solution.cost < best_solution.cost) {
                best_solution = current_solution;
            }
            add_to_tabu_list(current_solution.solution, tabu_list, &tabu_index, size);
        }

        for (int i = 0; i < num_neighbors; i++) {
            if (neighbors[i] != best_neighbor.solution) {
                free(neighbors[i]);
            }
        }
        free(neighbors);
    }

    printf("Best solution cost: %d\n", best_solution.cost);
    free(best_solution.solution);
    for (int i = 0; i < TABU_TENURE; i++) {
        free(tabu_list[i]);
    }
    free(tabu_list);
}

int evaluate_solution(int *solution, int size) {
    // Implement the evaluation function for the solution
    return 0;
}

void generate_neighbors(int *solution, int size, int **neighbors, int num_neighbors) {
    // Implement the function to generate neighboring solutions
}

bool is_tabu(int *solution, int **tabu_list, int tabu_size, int size) {
    for (int i = 0; i < tabu_size; i++) {
        bool is_same = true;
        for (int j = 0; j < size; j++) {
            if (solution[j] != tabu_list[i][j]) {
                is_same = false;
                break;
            }
        }
        if (is_same) {
            return true;
        }
    }
    return false;
}

void add_to_tabu_list(int *solution, int **tabu_list, int *tabu_index, int size) {
    for (int i = 0; i < size; i++) {
        tabu_list[*tabu_index][i] = solution[i];
    }
    *tabu_index = (*tabu_index + 1) % TABU_TENURE;
}
