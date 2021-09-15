#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "EBStop.hpp"
#include "Graph.hpp"
#include "IEStop.hpp"

// const need to be sent from argv
// #define COMP_INSTANCES 2
// #define STOR_INSTANCES 2
#define ALPHA 0.05
#define DELTA 0.05
// #define GRAPH_DIR "./"
// #define NUM_VERTEX 7115
// #define NUM_SAMPLING 3000

#define TASK_TAG 0
#define SAMPLING_TAG 1
#define ESTIMATION_TAG 2

#define DEBUG

struct Samplepara {
    int m;
    int source;
    Graph *g;
};

/*
pthread
sampling and send (non-blocking) zipped graph to computation process
*/
void *sampling(void *Param) {
    Samplepara *sa = (Samplepara *)Param;
    Graph *g = sa->g;
    int *subgraph = g->sample(sa->m);
    int m = subgraph[0];
    int n = subgraph[1];
    int size = 2 * m + n + 3;
#ifdef DEBUG
    printf("Sampling process send graph (of size %d) to compute process no %d:",
           size, sa->source);
    // for (int i = 0; i < size; i++) {
    //     printf(" %d", subgraph[i]);
    // }
    printf("\n");
#endif
    MPI_Request request;
    MPI_Isend(subgraph, size, MPI_INT, sa->source, SAMPLING_TAG, MPI_COMM_WORLD,
              &request);  // or mpi isend?
    // return (void *)&m;
}

/*
argv:
*/
int main(int argc, char **argv) {
    if (argc != 7) {
        printf(
            "args: COMP_INSTANCES, STOR_INSTANCES, GRAPH_DIR, NUM_VERTEX, "
            "NUM_SAMPLING, SAMPLE_TASK\n");
        return 0;
    }
    const int COMP_INSTANCES = atoi(argv[1]);
    const int STOR_INSTANCES = atoi(argv[2]);
    std::string GRAPH_DIR = argv[3];
    const int NUM_VERTEX = atoi(argv[4]);
    const int NUM_SAMPLING = atoi(argv[5]);
    std::string task = std::string(argv[6]);
    int my_rank;
    int provided;
    bool single_thread = false;
    MPI_Request request;
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        printf("MPI do not Support Multiple thread\n");
        single_thread = true;
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == 0) 
    {
        /*
        Main process,
        send (sampling) Work No. to each Computation process
        receive results from every work and determine whether stop
        */
        double time;
        bool ended = false;
        int work_no = 1;
        int workmap[COMP_INSTANCES + 1];
        int dst = 1;
        double estimation;
        int stopbuf[2] = {0};
        IEStop::get_instance().init(ALPHA, DELTA);
        for (; work_no <= COMP_INSTANCES; work_no++) {
            workmap[dst] = work_no;
            MPI_Send(&work_no, 1, MPI_INT, dst, TASK_TAG, MPI_COMM_WORLD);
            dst++;
        }
        time = MPI_Wtime();
        while (1) {
            double result[2];
            MPI_Recv(&result, 2, MPI_DOUBLE, MPI_ANY_SOURCE, ESTIMATION_TAG,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            dst = (int)std::round(result[0]);
            estimation = result[1];
#ifdef DEBUG
            printf(
                "Main process received result: %f from process %d, task %d.\n",
                estimation, dst, workmap[dst]);
#endif
            if (ended) {
                workmap[dst] = 0;
            } else {
                if (IEStop::get_instance().add(workmap[dst], estimation) == 0) {
                    IEStop::get_instance().print_res();
                    ended = true;
                    time = MPI_Wtime() - time;
                    printf("Full wall time = %f ms\n", time * 1000);
                }
                workmap[dst] = work_no++;
            }
            bool flag = true;  // all exit
            for (int i = 1; i <= COMP_INSTANCES; i++) {
                if (workmap[i] != 0) {
                    flag = false;
                }
            }
#ifdef DEBUG
            printf("Main process send task %d to process %d.\n", workmap[dst],
                   dst);
#endif
            MPI_Send(&workmap[dst], 1, MPI_INT, dst, TASK_TAG, MPI_COMM_WORLD);
            if (flag) {
                for (int i = COMP_INSTANCES + 1;
                     i <= COMP_INSTANCES + STOR_INSTANCES; i++) {
                    MPI_Send(stopbuf, 2, MPI_INT, i, SAMPLING_TAG,
                             MPI_COMM_WORLD);
                }
                MPI_Finalize();
                exit(0);
            }
        }
    } 
    else if (my_rank <= COMP_INSTANCES) 
    {
        /*
        computation process
        receive work no. from main process
        send sample size to each storage process
        receive sample, combine them and count the estimated result
        */
        int *arr =
            (int *)calloc(STOR_INSTANCES * 2, sizeof(int));  // random sizes
        double resultbuf[2] = {0};
        double time;
        while (1) {
            int work_no;
            MPI_Recv(&work_no, 1, MPI_INT, 0, TASK_TAG, MPI_COMM_WORLD,
                     &status);
#ifdef DEBUG
            printf("Compute process %d received task %d.\n", my_rank, work_no);
#endif
            if (work_no == 0) {
                MPI_Finalize();
                exit(0);
            }
            memset(arr, 0, 2 * STOR_INSTANCES * sizeof(int));
            srand(rand());
            for (int i = 0; i < NUM_SAMPLING; i++) {
                arr[(rand() % STOR_INSTANCES) * 2]++;
            }
#ifdef DEBUG
            printf("Compute process %d send sampling sizes:", my_rank);
            for (int i = 0; i < STOR_INSTANCES; i++) {
                printf(" %d", arr[i * 2]);
            }
            printf("\n");
#endif
            for (int i = 0; i < STOR_INSTANCES; i++) {
                arr[i * 2 + 1] = my_rank;
                MPI_Isend(&arr[i * 2], 2, MPI_INT, COMP_INSTANCES + i + 1,
                          SAMPLING_TAG, MPI_COMM_WORLD, &request);
            }
            time = MPI_Wtime();
            Graph graph = Graph();  // new sampling graph
            graph.init(NUM_VERTEX, NUM_SAMPLING);
            for (int i = 0; i < STOR_INSTANCES; i++) {
                int size;
                MPI_Probe(MPI_ANY_SOURCE, SAMPLING_TAG, MPI_COMM_WORLD,
                          &status);
                MPI_Get_count(&status, MPI_INT, &size);
#ifdef DEBUG
                printf("Compute process %d probed sampling of size %d.\n",
                       my_rank, size);
#endif
                int *buf = (int *)malloc(size * sizeof(int));
                MPI_Recv(buf, size, MPI_INT, status.MPI_SOURCE, SAMPLING_TAG,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#ifdef DEBUG
                printf("Compute process %d received Zipped graph: ", my_rank);
                // for (int i = 0; i < size; i++) {
                //     printf("%d ", buf[i]);
                // }
                printf("\n");
#endif
                if (graph.join(buf) != 0) {
                    printf("Join graph failed.\n");
                }
                free(buf);
            }
            time = MPI_Wtime() - time;
#ifdef DEBUG
            printf("Conpute process %d receive graph time = %f ms\n", my_rank,
                   time * 1000);
#endif
            time = MPI_Wtime();
            double result = 0;
            if (task == "triangle") {
                result = graph.count_triangle();
            } else if (task == "threechain") {
                result = graph.count_three_chain();
            } else if (task == "threemotif") {
                result = graph.count_three_motif();
            } else if (task == "fourchain") {
                result = graph.count_four_chain();
            } else if (task == "fivestar") {
                result = graph.count_five_star();
            } else {
                result = graph.count();
            }
            time = MPI_Wtime() - time;
#ifdef DEBUG
            printf("Conpute process %d compute time = %f ms\n", my_rank,
                   time * 1000);
#endif
            resultbuf[0] = my_rank;
            resultbuf[1] = result;
#ifdef DEBUG
            printf("Compute process %d: estimation result %f.\n", my_rank,
                   result);
#endif
            MPI_Send(resultbuf, 2, MPI_DOUBLE, 0, ESTIMATION_TAG,
                     MPI_COMM_WORLD);
        }
    } 
    else 
    {
        /*
        storage process
        read from file of splited graph
        receive sample size and do sampling, then send back
        in different thread
        */
        double time;
        time = MPI_Wtime();
        Graph graph = Graph();
        pthread_t threads[COMP_INSTANCES];
        Samplepara sa[COMP_INSTANCES];
        bool *threadinit = (bool *)calloc(COMP_INSTANCES, sizeof(bool));
        std::string str =
            GRAPH_DIR + '/' + std::to_string(my_rank - COMP_INSTANCES - 1);
        // printf("storage process %d read graph %s\n", my_rank, str.c_str());
        graph.init_from_file(str.c_str());
#ifdef DEBUG
        printf("storage process %d read graph %s done.\n", my_rank,
               str.c_str());
        time = MPI_Wtime() - time;
        printf("Load dataset time = %f ms\n", time * 1000);
#endif
        int samplingbuf[2] = {0};
        while (1) {
            MPI_Recv(samplingbuf, 2, MPI_INT, MPI_ANY_SOURCE, SAMPLING_TAG,
                     MPI_COMM_WORLD, &status);
            int m = samplingbuf[0];
            int source = samplingbuf[1];
#ifdef DEBUG
            printf(
                "Store process %d received sampling sizes %d from process %d\n",
                my_rank, m, source);
#endif
            if (source == 0) {
                MPI_Finalize();
                exit(0);
            }
            sa[source - 1].source = source;
            sa[source - 1].m = m;
            sa[source - 1].g = &graph;
            if (single_thread) {
                sampling((void *)&(sa[source - 1]));
            } else {
                if (threadinit[source - 1]) {
                    pthread_join(threads[source - 1], NULL);
                } else {
                    threadinit[source - 1] = true;
                }
                pthread_create(&(threads[source - 1]), NULL, sampling,
                               (void *)&(sa[source - 1]));
            }
        }
    }
    return 0;
}
