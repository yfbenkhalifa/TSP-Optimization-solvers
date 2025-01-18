// cplex_builder.h
#ifndef CPLEX_BUILDER_H
#define CPLEX_BUILDER_H

#include "common.h"
#include <cplex.h>
#include "tsp_common.h"

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

// Graph structure
typedef struct Graph {
    int numVertices;
    Node** adjLists; // Array of adjacency lists
    int* visited;    // Array for tracking visited nodes
} Graph;

typedef struct {
    int nnodes;
    int* solution;
}cplex_tsp_solution;

typedef struct {
    int error_code;
    int* solution;
}cplex_tsp_result;


Node* createNode(int vertex);
Graph* createGraph(int nnodes);
void addEdge(Graph* graph, int src, int dest);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
double* TSPopt(instance *inst, CPXENVptr env, CPXLPptr lp);
void printGraph(Graph* graph);
double dist(int i, int j, instance *inst);
void build_solution(double *xstar, instance *instance, int *succ, int *comp, int *ncomp);
int get_cplex_variable_index(int i, int j, instance* inst);
void findConnectedComponents(Graph* graph);
void findConnectedComponentsFromSolution(int *solution, int nnodes, int *component_map);
int add_bender_constraint(const int* component_map, instance* instance, CPXENVptr env, CPXLPptr lp, int ncomponents);
void add_bender_constraint_from_component(int *component, instance *instance, CPXENVptr env, CPXLPptr lp, int component_length);
int cplex_tsp_branch_and_cut(instance *instance,  int *solution, int _verbose);
#endif // CPLEX_BUILDER_H