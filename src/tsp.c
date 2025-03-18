//
// Created by wiz on 3/29/24.
//

#include "tsp.h"


#define VERBOSE 50

void init_solution(instance* inst, int* solution)
{
    for (int i = 0; i < inst->nnodes; i++)
    {
        solution[i] = -1;
    }
}

bool is_neighbor(const int* solution1, const int* solution2, int size)
{
    int differing_edges = 0;
    for (int i = 0; i < size; i++)
    {
        if (solution1[i] != solution2[i])
        {
            differing_edges++;
        }
    }
    return differing_edges == 2;
}

void two_opt_swap(int* solution, int size, Edge e1, Edge e2)
{
    int* solution_copy = (int*)malloc(sizeof(int) * size);
    solution[e1.node1] = e2.node1;
    int i = e1.node2;
    int temp = -1;
    memcpy(solution_copy, solution, size * sizeof(int));
    while (i != e2.node1)
    {
        temp = solution[i];
        solution_copy[temp] = i;
        i = temp;
    }
    memcpy(solution, solution_copy, size * sizeof(int));
    solution[e1.node2] = e2.node2;
}

double tsp_two_opt(instance* instance, Solution *solution)
{
    double deltaCost = 0;
    int min_a1 = -1;
    int min_a2 = -1;
    int min_b1 = -1;
    int min_b2 = -1;

    for (int i = 0; i < instance->nnodes; i++)
    {
        for (int j = 0; j < instance->nnodes; j++)
        {
            if (i == j) continue;
            int a1 = i;
            int a2 = solution->solution[i];
            int b1 = j;
            int b2 = solution->solution[j];

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
        Edge e1, e2;
        e1.node1 = min_a1;
        e1.node2 = min_a2;
        e2.node1 = min_b1;
        e2.node2 = min_b2;
        two_opt_swap(solution->solution, instance->nnodes, e1, e2);
    }

    return deltaCost;
}

void tsp_grasp(instance* inst, int starting_node)
{
    clock_t start_time = clock();
    log_message(LOG_LEVEL_INFO, "Solving TSP with GRASP\n");
    log_message(LOG_LEVEL_INFO, "Instance details: nnodes = %d\n", inst->nnodes);
    for (int i = 0; i < inst->nnodes; i++)
    {
        log_message(LOG_LEVEL_INFO, "Node %d: (%f, %f)\n", i, inst->xcoord[i], inst->ycoord[i]);
    }
    log_message(LOG_LEVEL_INFO, "Starting GRASP with starting node %d\n", starting_node);

    int current_node_index = starting_node;
    int remaining_nodes_count = inst->nnodes;
    solution nearest_node;
    int* remaining_nodes = (int*)malloc(inst->nnodes * sizeof(int));

    inst->solution = (int*)malloc(inst->nnodes * sizeof(int));
    inst->best_cost_value = 0;

    for (int i = 0; i < inst->nnodes; i++)
    {
        inst->solution[i] = -1;
        remaining_nodes[i] = i;
    }

    remaining_nodes[current_node_index] = remaining_nodes[--remaining_nodes_count];

    for (int i = 0; i < inst->nnodes; i++)
    {
        log_message(LOG_LEVEL_INFO, "Current node index: %d\n", current_node_index);
        nearest_node = euclidean_nearest_node(inst, current_node_index, remaining_nodes, &remaining_nodes_count);

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

    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    inst->elapsed_time = elapsed_time;
    inst->best_cost_value = compute_solution_cost(inst, inst->solution);
    log_message(LOG_LEVEL_INFO, "GRASP solution time: %f seconds\n", elapsed_time);
    log_message(LOG_LEVEL_INFO, "GRASP solution cost: %f\n", inst->best_cost_value);

    free(remaining_nodes);
}

void random_solution(instance* inst, int* solution)
{
    // Initialize the solution with node indices
    for (int i = 0; i < inst->nnodes; i++)
    {
        solution[i] = i;
    }

    // Shuffle the solution array to create a random solution
    srand(time(NULL));
    for (int i = inst->nnodes - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = solution[i];
        solution[i] = solution[j];
        solution[j] = temp;
    }

    // Ensure the solution is a valid TSP solution
    if (!is_tsp_solution(inst, solution))
    {
        random_solution(inst, solution); // Retry if the solution is not valid
    }
}

bool is_2opt_neighbour(int* solution1, int* solution2, int size)
{
    int differing_edges = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            Edge e1, e2;
            e1.node1 = i;
            e1.node2 = solution1[i];
            e2.node1 = j;
            e2.node2 = solution1[j];

            int* solution2_modified = (int*)malloc(size * sizeof(int));
            memcpy(solution2_modified, solution1, size * sizeof(int));
            two_opt_swap(solution2_modified, size, e1, e2);

            if (compare_solutions(solution2_modified, solution2, size))
            {
                differing_edges += 2;
            }
            free(solution2_modified);
        }
    }
    return differing_edges == 2;
}

void tsp_extra_mileage(instance* inst, pair starting_pair)
{
    clock_t start_time = clock();
    log_message(LOG_LEVEL_INFO, "Solving TSP with Extra Mileage\n");
    log_message(LOG_LEVEL_INFO, "Instance details: nnodes = %d\n", inst->nnodes);

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
        log_message(LOG_LEVEL_INFO, "Covered nodes count: %d\n", state.covered_nodes_count);
        solution best_node;
        for (int i = 0; i < state.covered_nodes_count; i++)
        {
            log_message(LOG_LEVEL_INFO, "Current node index: %d\n", i);
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
                log_message(LOG_LEVEL_INFO, "Best node found: %d\n", best_node.node);
                inst->solution[current_node] = best_node.node;
                inst->solution[best_node.node] = current_node_opposite;
                state.covered_nodes[state.covered_nodes_count++] = best_node.node;
                state.uncovered_nodes[best_node.node_index] = state.uncovered_nodes[--state.uncovered_nodes_count];
            }
        }
    }

    inst->best_cost_value = compute_solution_cost(inst, inst->solution);
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    inst->elapsed_time = elapsed_time;
    log_message(LOG_LEVEL_INFO, "Extra Mileage solution time: %f seconds\n", elapsed_time);
    log_message(LOG_LEVEL_INFO, "Extra Mileage solution cost: %f\n", inst->best_cost_value);
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

void tabu_search(instance* inst, int* initial_solution, int size)
{
    clock_t start_time = clock();
    log_message(LOG_LEVEL_INFO, "Solving TSP with Tabu Search\n");
    log_message(LOG_LEVEL_INFO, "Current best known solution cost: %f\n", inst->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Instance details: nnodes = %d\n", inst->nnodes);


    int** tabu_list = (int**)malloc(TABU_TENURE * sizeof(int*));
    for (int i = 0; i < TABU_TENURE; i++)
    {
        tabu_list[i] = (int*)malloc(size * sizeof(int));
    }
    int tabu_index = 0;

    Solution best_solution;
    best_solution.solution = (int*)malloc(size * sizeof(int));
    memcpy(best_solution.solution, initial_solution, size * sizeof(int));
    best_solution.cost = compute_solution_cost(inst, best_solution.solution);

    Solution current_solution = best_solution;

    for (int iter = 0; iter < MAX_ITERATIONS; iter++)
    {
        int num_neighbors = size;
        int** neighbours = (int**)malloc(num_neighbors * sizeof(int*));
        for (int i = 0; i < num_neighbors; i++)
        {
            neighbours[i] = (int*)malloc(size * sizeof(int));
        }
        generate_neighbors(current_solution.solution, size, neighbours, num_neighbors);

        Solution best_neighbor;
        best_neighbor.solution = NULL;
        best_neighbor.cost = INT_MAX;

        for (int i = 0; i < num_neighbors; i++)
        {
            int cost = evaluate_solution(neighbours[i], size);
            if (cost < best_neighbor.cost && !is_tabu(neighbours[i], tabu_list, TABU_TENURE, size))
            {
                best_neighbor.solution = neighbours[i];
                best_neighbor.cost = cost;
            }
        }

        if (best_neighbor.solution != NULL)
        {
            current_solution = best_neighbor;
            if (current_solution.cost < best_solution.cost)
            {
                best_solution = current_solution;
            }
            add_to_tabu_list(current_solution.solution, tabu_list, &tabu_index, size);
        }

        for (int i = 0; i < num_neighbors; i++)
        {
            if (neighbours[i] != best_neighbor.solution)
            {
                free(neighbours[i]);
            }
        }
        free(neighbours);
    }

    inst->best_cost_value = compute_solution_cost(inst, best_solution.solution);
    log_message(LOG_LEVEL_INFO, "Best solution cost: %d\n", best_solution.cost);
    memcpy(inst->solution, best_solution.solution, size * sizeof(int));

    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    inst->elapsed_time = elapsed_time;
    log_message(LOG_LEVEL_INFO, "Tabu Search solution time: %f seconds\n", elapsed_time);
    log_message(LOG_LEVEL_INFO, "Tabu Search solution cost: %f\n", best_solution.cost);

    free(best_solution.solution);
    for (int i = 0; i < TABU_TENURE; i++)
    {
        free(tabu_list[i]);
    }
    free(tabu_list);
}

void tsp_vns(instance* inst, int* initial_solution, int neighbourhood_size)
{
    // initializ time and log
    clock_t start_time = clock();
    log_message(LOG_LEVEL_INFO, "Solving TSP with VNS\n");
    log_message(LOG_LEVEL_INFO, "Current best known solution cost: %f\n", inst->best_cost_value);
    log_message(LOG_LEVEL_INFO, "Instance details: nnodes = %d\n", inst->nnodes);

    int* current_solution = (int*)malloc(inst->nnodes * sizeof(int));
    memcpy(current_solution, initial_solution, inst->nnodes * sizeof(int));
    bool stop_criterion = true;
    int iterations_without_improvement = 0;
    const int max_iterations = 100;
    const int time_limit_milliseconds = 3600;
    const int max_stuck_iterations = 10;
    double best_solution_cost = compute_solution_cost(inst, current_solution);
    do
    {
        log_message(LOG_LEVEL_INFO, "Iteration %d\n", iterations_without_improvement);
        double current_solution_cost = compute_solution_cost(inst, current_solution);
        log_message(LOG_LEVEL_INFO, "Current solution cost: %f\n", current_solution_cost);
        int** neighbourhood = (int**)malloc(neighbourhood_size * sizeof(int*));
        log_message(LOG_LEVEL_INFO, "Generating new neighbour\n");
        generate_neighbors(current_solution, inst->nnodes, neighbourhood, neighbourhood_size);
        int *selected_neighbour = (int*)malloc(inst->nnodes * sizeof(int));
        if (neighbourhood_size == 1)
        {
            memcpy(selected_neighbour, neighbourhood, inst->nnodes * sizeof(int));
        }
        else
        {
            int random_index = rand() % neighbourhood_size;
            memcpy(selected_neighbour, neighbourhood[random_index], inst->nnodes * sizeof(int));
        }
        // check new neighbour cost
        double new_solution_cost = compute_solution_cost(inst, selected_neighbour);

        log_message(LOG_LEVEL_INFO, "New neighbour cost: %f\n", new_solution_cost);
        if (new_solution_cost < current_solution_cost)
        {
            log_message(LOG_LEVEL_INFO, "Better solution found...\n");
            memcpy(current_solution, selected_neighbour, inst->nnodes * sizeof(int));
            best_solution_cost = new_solution_cost;
            iterations_without_improvement = 0;
        }
        else
        {
            iterations_without_improvement++;
        }


        double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

        if (time_limit_milliseconds > 0 && elapsed_time > time_limit_milliseconds)
        {
            log_message(LOG_LEVEL_INFO, "Time limit reached\n");
            stop_criterion = false;
        }

        if (max_iterations > 0 && iterations_without_improvement > max_iterations)
        {
            log_message(LOG_LEVEL_INFO, "Max iterations reached\n");
            stop_criterion = false;
        }
    }
    while (stop_criterion);

    // update the instance with the new solution
    memcpy(inst->solution, current_solution, inst->nnodes * sizeof(int));

    // update elapsed time
    double total_elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

    //update incumbent solution cost
    inst->best_cost_value = best_solution_cost;

    // log the results
    log_message(LOG_LEVEL_INFO, "VNS solution time: %f seconds\n", total_elapsed_time);
    log_message(LOG_LEVEL_INFO, "VNS solution cost: %f\n", inst->best_cost_value);
}

int evaluate_solution(int* solution, int size)
{
    // Implement the evaluation function for the solution
    return 0;
}

void generate_neighbors(int* solution, int size, int** neighbors, int num_neighbors)
{
    for (int i = 0; i < num_neighbors; i++)
    {
        neighbors[i] = (int*)malloc(size * sizeof(int));
        memcpy(neighbors[i], solution, size * sizeof(int));
        int index1 = rand() % size;
        int index2 = rand() % size;
        while (index1 == index2)
        {
            index2 = rand() % size;
        }
        Edge e1, e2;
        e1.node1 = index1;
        e1.node2 = solution[index1];
        e2.node1 = index2;
        e2.node2 = solution[index2];
        two_opt_swap(neighbors[i], size, e1, e2);
    }
}

bool is_tabu(int* solution, int** tabu_list, int tabu_size, int size)
{
    for (int i = 0; i < tabu_size; i++)
    {
        bool is_same = true;
        for (int j = 0; j < size; j++)
        {
            if (solution[j] != tabu_list[i][j])
            {
                is_same = false;
                break;
            }
        }
        if (is_same)
        {
            return true;
        }
    }
    return false;
}

void add_to_tabu_list(int* solution, int** tabu_list, int* tabu_index, int size)
{
    for (int i = 0; i < size; i++)
    {
        tabu_list[*tabu_index][i] = solution[i];
    }
    *tabu_index = (*tabu_index + 1) % TABU_TENURE;
}
