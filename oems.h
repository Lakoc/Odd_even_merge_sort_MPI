#ifndef OEMS_H
#define OEMS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <assert.h>

#define INPUT_SIZE 8
#define N_NODES 19
#define ELEMENTS_PER_PROCESS 2
#define NEXT_LEVEL_NEIGHBOURS 2
#define SINGLE_ELEMENT 1

#define ROOT_NODE 0
#define BLOCK1_SIZE 4
#define BLOCK2_SIZE 6
#define BLOCK3_SIZE 9

#define DEFAULT_TAG 1
#define LOWER_TAG 10
#define HIGHER_TAG 11

#define DEBUG false
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

// Each processor tag to send info to next processor
u_int8_t tags[N_NODES][ELEMENTS_PER_PROCESS] = {{LOWER_TAG,  LOWER_TAG},
                                                {HIGHER_TAG, HIGHER_TAG},
                                                {LOWER_TAG,  LOWER_TAG},
                                                {HIGHER_TAG, HIGHER_TAG},
                                                {LOWER_TAG,  LOWER_TAG},
                                                {HIGHER_TAG, LOWER_TAG},
                                                {HIGHER_TAG, LOWER_TAG},
                                                {HIGHER_TAG, HIGHER_TAG},
                                                {LOWER_TAG,  LOWER_TAG},
                                                {HIGHER_TAG, HIGHER_TAG},
                                                {0,          LOWER_TAG},
                                                {HIGHER_TAG, LOWER_TAG},
                                                {HIGHER_TAG, LOWER_TAG},
                                                {HIGHER_TAG, 7},
                                                {LOWER_TAG,  LOWER_TAG},
                                                {HIGHER_TAG, HIGHER_TAG},
                                                {1,          2},
                                                {3,          4},
                                                {5,          6},
};

// Edges to the next processors
int edges[N_NODES * ELEMENTS_PER_PROCESS] = {4, 5,      // 1x1[0] 0
                                             4, 5,      // 1x1[1] 1
                                             6, 7,      // 1x1[2] 2
                                             6, 7,      // 1x1[3] 3
                                             10, 8,     // 2x2[0,0] 4
                                             8, 13,   // 2x2[0,1] 5
                                             10, 9,   // 2x2[0,2] 6
                                             9, 13,   // 2x2[0,3] 7
                                             12, 11,  // 2x2[1,0] 8
                                             12, 11,  // 2x2[1,1] 9
                                             0, 14,   // 4x4[0,0] 10
                                             14, 18,  // 4x4[0,1] 11
                                             16, 15,  // 4x4[0,2] 12
                                             15, 0,   // 4x4[0,3] 13
                                             16, 17,  // 4x4[1,0] 14
                                             17, 18,  // 4x4[1,1] 15
                                             0, 0,    // 4x4[2,0] 16
                                             0, 0,    // 4x4[2,1] 17
                                             0, 0,    // 4x4[2,2] 18
};

void sort(u_int8_t input[]);

u_int8_t *read_input_from_file(char *path);

void print_input_data(u_int8_t *data);

void print_output_data(u_int8_t *data);

void create_sorting_graph(MPI_Comm *graphComm);

int *load_neighbors(int rank, MPI_Comm graphComm);

void gatherToTheRootNode(u_int8_t *output);

void mpiError();

#endif //OEMS_H
