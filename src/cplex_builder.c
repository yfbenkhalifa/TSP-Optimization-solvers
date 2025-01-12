//
// Created by WiZ on 13/11/2024.
//

#include "cplex_builder.h"

#define EPS 1e-5
// Function to create a node
Node *createNode(int vertex) {
    Node *newNode = (Node *) malloc(sizeof(Node));
    newNode->vertex = vertex;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph
Graph *createGraph(int vertices) {
    Graph *graph = (Graph *) malloc(sizeof(Graph));
    graph->numVertices = vertices;

    // Create adjacency lists and visited array
    graph->adjLists = (Node **) malloc(vertices * sizeof(Node *));
    graph->visited = (int *) malloc(vertices * sizeof(int));

    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL; // Initialize adjacency lists as empty
        graph->visited[i] = 0; // Initialize visited array to "not visited"
    }

    return graph;
}

// Add an undirected edge to the graph
void addEdge(Graph *graph, int src, int dest) {
    // Check to avoid duplicates (optional for safety)
    Node *temp = graph->adjLists[src];
    while (temp != NULL) {
        if (temp->vertex == dest) {
            return; // Edge already exists, ignore
        }
        temp = temp->next;
    }

    // Add dest to src's adjacency list
    Node *newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    // Add src to dest's adjacency list (undirected graph)
    newNode = createNode(src);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

// Function to populate the graph using a solution vector
Graph *populateGraph(int edges[][2], int edgeCount) {
    // Find the number of nodes dynamically (max node ID + 1)
    int maxNode = 0;
    for (int i = 0; i < edgeCount; i++) {
        if (edges[i][0] > maxNode) maxNode = edges[i][0];
        if (edges[i][1] > maxNode) maxNode = edges[i][1];
    }
    int numVertices = maxNode + 1;

    // Create the graph
    Graph *graph = createGraph(numVertices);

    // Add edges to the graph
    for (int i = 0; i < edgeCount; i++) {
        addEdge(graph, edges[i][0], edges[i][1]);
    }

    return graph;
}

// DFS to find connected components
void dfs(Graph *graph, int vertex, int *component, int *componentSize) {
    graph->visited[vertex] = 1; // Mark node as visited
    component[(*componentSize)++] = vertex; // Add vertex to the connected component

    Node *adjList = graph->adjLists[vertex];
    Node *temp = adjList;

    // Traverse all its neighbors
    while (temp != NULL) {
        int connectedVertex = temp->vertex;
        if (!graph->visited[connectedVertex]) {
            dfs(graph, connectedVertex, component, componentSize);
        }
        temp = temp->next;
    }
}

// Function to find connected components in the graph
void findConnectedComponents(Graph *graph) {
    int *component = (int *) malloc(graph->numVertices * sizeof(int)); // Temporary array for a component

    printf("Connected components:\n");
    for (int i = 0; i < graph->numVertices; i++) {
        if (!graph->visited[i]) {
            int componentSize = 0;

            // Perform DFS starting from this vertex to find all nodes in the current connected component
            dfs(graph, i, component, &componentSize);

            // Print the discovered connected component
            printf("Component: ");
            for (int j = 0; j < componentSize; j++) {
                printf("%d ", component[j]);
            }
            printf("\n");
        }
    }

    free(component);
}

void findConnectedComponentsFromSolution(int *solution, int nnodes, int *component_map) {
    bool *visited = (bool *) calloc(nnodes, sizeof(bool)); // Keep track of visited nodes
    int currentComponent = 0; // Current component index

    for (int i = 0; i < nnodes; i++) {
        component_map[i] = -1;
    }


    free(visited);
}


// Print adjacency list of the graph (for verification/debugging)
void printGraph(Graph *graph) {
    printf("Graph adjacency list:\n");
    for (int i = 0; i < graph->numVertices; i++) {
        printf("Vertex %d: ", i);
        Node *temp = graph->adjLists[i];
        while (temp) {
            printf("%d -> ", temp->vertex);
            temp = temp->next;
        }
        printf("NULL\n");
    }
}

// Free memory allocated for the graph
void freeGraph(Graph *graph) {
    for (int i = 0; i < graph->numVertices; i++) {
        Node *temp = graph->adjLists[i];
        while (temp) {
            Node *toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph->adjLists);
    free(graph->visited);
    free(graph);
}

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if (!inst->integer_costs) return sqrt(dx * dx + dy * dy);
    int dis = sqrt(dx * dx + dy * dy) + 0.499999999; // nearest integer
    return dis + 0.0;
}

int get_cplex_variable_index(int i, int j, instance *inst) // to be verified
{
    if (i == j) print_error(" i == j in xpos");
    if (i > j) return get_cplex_variable_index(j, i, inst);
    int pos = i * inst->nnodes + j - ((i + 1) * (i + 2)) / 2;
    return pos;
}

int VERBOSE = 10000;

void build_solution(double *xstar, instance *instance, int *succ, int *comp, int *ncomp) {
#ifdef  DEBUG

    int *degree = (int *) calloc(instance->nnodes, sizeof(int));
    for (int i=0; i < instance->nnodes; i++) {
        for (int j = i+1 ; j < instance->nnodes; j++) {
            int xpos = get_cplex_variable_index(i, j, instance)
            if (fabs(xstar[xpos]) > EPS && fabs(xstar[xpos]-1.0) > EPS) print_error("ERROR: invalid value for xpos")

            if (xstar[xpos] > 0.5) {
                ++degree[i];
                ++degree[j];
            }
        }
    }
    for (int i=0; i < instance->nnodes; i++) {
        if (degree[i] != 2) print_error("ERROR: invalid solution")
    }

#endif

    *ncomp = 0;
    for (int i = 0; i < instance->nnodes; i++) {
        succ[i] = -1;
        comp[i] = -1;
    }

    for (int start = 0; start < instance->nnodes; start++) {
        if (comp[start] >= 0) continue;

        (*ncomp)++;
        int i = start;
        bool done = false;
        while (!done) {
            comp[i] = *ncomp;
            done = true;
            for (int j = 0; j < instance->nnodes; j++) {
                if (i != j && xstar[get_cplex_variable_index(i, j, instance)] > 0.5 && comp[j] == -1) {
                    succ[i] = j;
                    i = j;
                    done = false;
                    break;
                }
            }
        }
        succ[i] = start;
    }
}


void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    double zero = 0.0;
    char binary = 'B';

    char **cname = (char **) calloc(1, sizeof(char *)); // (char **) required by cplex...
    cname[0] = (char *) calloc(100, sizeof(char));

    // add binary var.s x(i,j) for i < j

    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            sprintf(cname[0], "x(%d,%d)", i + 1, j + 1); // ... x(1,2), x(1,3) ....
            double cost_function_value = dist(i, j, inst); // cost == distance
            double lower_bound = 0.0;
            double upper_bound = 1.0;
            if (CPXnewcols(env, lp, 1, &cost_function_value, &lower_bound, &upper_bound, &binary, cname)) {
                print_error(" wrong CPXnewcols on x var.s");
            }
            if (CPXgetnumcols(env, lp) - 1 != get_cplex_variable_index(i, j, inst)) {
                print_error(" wrong position for x var.s");
            }
        }
    }

    int *index = (int *) calloc(inst->nnodes, sizeof(int));
    double *value = (double *) calloc(inst->nnodes, sizeof(double));

    for (int h = 0; h < inst->nnodes; h++) // add the degree constraint on node h
    {
        double rhs = 2.0;
        char sense = 'E'; // 'E' for equality constraint
        sprintf(cname[0], "degree(%d)", h + 1);
        int non_zero_variables_count = 0;
        for (int i = 0; i < inst->nnodes; i++) {
            if (i == h) continue;
            index[non_zero_variables_count] = get_cplex_variable_index(i, h, inst);
            value[non_zero_variables_count] = 1.0;
            non_zero_variables_count++;
        }
        int izero = 0;
        if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &rhs, &sense, &izero, index, value, NULL, &cname[0])) {
            print_error("CPXaddrows(): error 1");
        }
    }

    free(value);
    free(index);

    free(cname[0]);
    free(cname);

    if (VERBOSE >= 100) CPXwriteprob(env, lp, "model.lp", NULL);
}

double* TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp) {
    // open CPLEX model
    int error;


    error = CPXmipopt(env, lp);
    if (error) {
        printf("CPX error code %d\n", error);
        print_error("CPXmipopt() error");
    }

    int ncols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(ncols, sizeof(double));
    int cpxgetx_error = CPXgetx(env, lp, xstar, 0, ncols - 1);

    if (cpxgetx_error > 0) print_error("CPXgetx() error");

    for (int i = 0; i < inst->nnodes; i++) {
        for (int j = i + 1; j < inst->nnodes; j++) {
            if (xstar[get_cplex_variable_index(i, j, inst)] > 0.5) {
                printf("  ... x(%3d,%3d) = 1\n", i + 1, j + 1);
            }
        }
    }

    return xstar;
}

void add_bender_constraint_from_component(int *component, instance *instance, CPXENVptr env, CPXLPptr lp, int component_length) {
    int constraint_rhs_value = component_length;


}


void add_bender_constraint(const int *component_map, instance *instance, CPXENVptr env, CPXLPptr lp, int ncomponents) {
    if (ncomponents == 1) return;

    double right_hand_side_value = 0.0;
    int ncols = CPXgetnumcols(env, lp);
    int *index = (int *) calloc(ncols, sizeof(int));
    double *coefficients = (double *) calloc(ncols, sizeof(double));
    int izero = 0;
    for (int k = 1; k <= ncomponents; k++) {
        int non_zero_variables_count = 0;
        right_hand_side_value = 0.0;
        char constraint_sense = 'L';
        for (int i = 0; i < instance->nnodes; i++) {
            if (component_map[i] != k) continue;
            right_hand_side_value++; // increase cardinality of the set
            for (int j = i + 1; j < instance->nnodes; j++) {
                if (component_map[j] != k) continue;
                index[non_zero_variables_count] = get_cplex_variable_index(i, j, instance);
                coefficients[non_zero_variables_count] = 1.0;
                non_zero_variables_count++;
            }
        }
        right_hand_side_value--;
        if (right_hand_side_value >= instance->nnodes) continue;
        if (CPXaddrows(env, lp, 0, 1, non_zero_variables_count, &right_hand_side_value, &constraint_sense, &izero, index,
                       coefficients, NULL, NULL)) {
            print_error("CPXaddrows(): error 1");
        }
    }

    free(index);
    free(coefficients);
}
