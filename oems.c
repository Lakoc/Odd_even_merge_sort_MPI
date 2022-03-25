#include "oems.h"

/*
 * Function:  sort
 * --------------------
 * Sorts pair of numbers.
 *
 *  input: a pair of two unsigned integers
 *
 *  returns: sorted tuple
 */
void sort(u_int8_t input[]) {
    if (input[0] > input[1]) {
        u_int8_t tmp = input[0];
        input[0] = input[1];
        input[1] = tmp;
    }
}

/*
 * Function:  read_input_from_file
 * --------------------
 * Read binary file on provided path containing N values (u_int8_t numbers), and store those values to the buffer.
 *
 *  path: file path
 *
 *  returns: buffer with N numbers
 */
u_int8_t *read_input_from_file(char *path) {
    u_int8_t *input = (u_int8_t *) malloc(sizeof(u_int8_t) * INPUT_SIZE);
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Could not open input file\n.");
        exit(1);
    }
    fread(input, sizeof(u_int8_t), INPUT_SIZE, fp);
    int ret = fclose(fp);
    if (ret != 0) {
        printf("Could not close input file\n.");
        exit(1);
    }
    return input;
}

/*
 * Function:  print_input_data
 * --------------------
 * Print numbers from buffer on single line separated by space.
 *
 *  data: buffer with numbers to be printed.
 */
void print_input_data(u_int8_t *data) {
    for (u_int8_t i = 0; i < INPUT_SIZE; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%u", data[i]);
    }
    printf("\n");
}

/*
 * Function:  print_output_data
 * --------------------
 * Print numbers from buffer separated by new line.
 *
 *  data: buffer with numbers to be printed
 */
void print_output_data(u_int8_t *data) {
    for (u_int8_t i = 0; i < INPUT_SIZE; i++) {
        printf("%u\n", data[i]);
    }
}


/*
 * Function:  create_sorting_graph
 * --------------------
 * Create graph representing architecture of Odd-Even Merge Sort for 8 numbers.
 *
 *  graphComm: pointer where new graph communicator should be created
 */
void create_sorting_graph(MPI_Comm *graphComm) {
    int indexes[N_NODES];
    for (int i = 0; i < N_NODES; ++i) {
        indexes[i] = NEXT_LEVEL_NEIGHBOURS * (i + 1);
    }
    if (MPI_Graph_create(MPI_COMM_WORLD, N_NODES, indexes, edges, false, graphComm)) { mpiError(); }
}

/*
 * Function:  load_neighbors
 * --------------------
 * Load each node neighbours.
 *
 *  rank: identification of the node
 *
 *  graphComm: pointer to the graph communicator
 *
 *  returns: buffer with N neighbours of the node rank
 */
int *load_neighbors(int rank, MPI_Comm graphComm) {
    int *neighbours = (int *) malloc(sizeof(int) * NEXT_LEVEL_NEIGHBOURS);
    if (MPI_Graph_neighbors(graphComm, rank, NEXT_LEVEL_NEIGHBOURS, neighbours)) { mpiError(); }
    return neighbours;
}

/*
 * Function:  gatherToTheRootNode
 * --------------------
 * Gather sorted sequence to the root node buffer.
 *
 *  output: buffer where sorted sequence should be saved
 */
void gatherToTheRootNode(u_int8_t *output) {
    for (u_int8_t i = 0; i < N_NODES; i++) {
        for (u_int8_t j = 0; j < ELEMENTS_PER_PROCESS; j++) {
            u_int8_t tag = tags[i][j];
            if (tag <= INPUT_SIZE) {
                if (MPI_Recv(&output[tag], 1, MPI_UINT8_T, i, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) { mpiError(); }
            }
        }
    }
}


void mpiError() {
    printf("MPI library operation failed.\n");
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
}

int main(int argc, char **argv) {

    int n_processes;
    int groups[3] = {BLOCK1_SIZE - 1, BLOCK2_SIZE - 1, BLOCK3_SIZE - 1};
    int group, rank;
    u_int8_t *input = NULL;

    if (MPI_Init(&argc, &argv)) { mpiError(); }
    MPI_Request send_request1, send_request2;

    // Load process counts and each process rank
    if (MPI_Comm_size(MPI_COMM_WORLD, &n_processes)) { mpiError(); }
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank)) { mpiError(); }

    if (n_processes != N_NODES) {
        printf("This application is meant to be run with %d MPI processes, not %d.\n", N_NODES, n_processes);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Split to 3 groups
    MPI_Comm blocks;
    group = (int) (rank > groups[0]) + (int) (rank > groups[1]);
    if (MPI_Comm_split(MPI_COMM_WORLD, group, rank, &blocks)) { mpiError(); }

    // Create a sorting graph
    MPI_Comm graphComm;
    create_sorting_graph(&graphComm);

    // Root node - Load input data
    if (rank == ROOT_NODE) {
        debug_print("[MPI process %d, block %d] Loading data...\n", rank, group);
        input = read_input_from_file("numbers");
        print_input_data(input);
    }

    // Define buffer for each process
    u_int8_t *rbuf = (u_int8_t *) malloc(sizeof(u_int8_t) * ELEMENTS_PER_PROCESS);

    if (group == 0) {
        // First block data are being distributed from the root node
        if (MPI_Scatter(input, ELEMENTS_PER_PROCESS, MPI_UINT8_T, rbuf, ELEMENTS_PER_PROCESS,
                        MPI_UINT8_T, ROOT_NODE, blocks)) { mpiError(); }
        free(input);
    } else {
        // Next blocks receive data from the precessing nodes
        debug_print("[MPI process %d, block %d] Waiting for first block to finish....\n", rank, group);
        if (MPI_Recv(rbuf, SINGLE_ELEMENT, MPI_UINT8_T, MPI_ANY_SOURCE, LOWER_TAG, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE)) { mpiError(); }
        if (MPI_Recv(rbuf + sizeof(u_int8_t), SINGLE_ELEMENT, MPI_UINT8_T, MPI_ANY_SOURCE, HIGHER_TAG, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE)) { mpiError(); }
    }

    debug_print("[MPI process %d, block %d] Data (%u, %u)\n",
                rank, group, rbuf[0], rbuf[1]);

    // Sort values at each node
    sort(rbuf);

    debug_print("[MPI process %d, block %d] Sorted data (%u, %u)\n",
                rank, group, rbuf[0], rbuf[1]);

    // Get node neighbours
    int *neighbours = load_neighbors(rank, graphComm);

    debug_print("[MPI process %d, block %d] Sending value %u to the node %d and %u to the node %d.\n", rank, group,
                rbuf[0], neighbours[0], rbuf[1], neighbours[1]);

    // Send non-blocking (otherwise deadlock may occur) message with data to the following nodes
    if (MPI_Isend(rbuf, SINGLE_ELEMENT, MPI_UINT8_T, neighbours[0], tags[rank][0], MPI_COMM_WORLD,
                  &send_request1)) { mpiError(); }
    if (MPI_Isend(rbuf + sizeof(u_int8_t), SINGLE_ELEMENT, MPI_UINT8_T, neighbours[1], tags[rank][1], MPI_COMM_WORLD,
                  &send_request2)) { mpiError(); }

    // Clean up memory
    free(neighbours);
    free(rbuf);

    debug_print("[MPI process %d, block %d] Done, barrier reached.\n", rank, group);

    // Wait for all processes to finish sorting
    if (MPI_Barrier(MPI_COMM_WORLD)) { mpiError(); }

    // Root node process output
    if (rank == ROOT_NODE) {
        u_int8_t output[INPUT_SIZE];
        debug_print("[MPI process %d, block %d] Collecting data over nodes.\n", rank, group);
        gatherToTheRootNode(output);
        debug_print("[MPI process %d, block %d] Printing sorted data.\n", rank, group);
        print_output_data(output);
    }

    if (MPI_Finalize()) { mpiError(); }

    return EXIT_SUCCESS;
}