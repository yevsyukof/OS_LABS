#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define num_steps 200000000

typedef struct {
    int begin_iter_idx;
    int count_iterations;
} initial_conditions;

void *thread_task(void *arg) {
    initial_conditions *initial_condition = (initial_conditions *) arg;
    double pi = 0;
    for (int i = initial_condition->begin_iter_idx;
         i < initial_condition->begin_iter_idx + initial_condition->count_iterations; ++i) {
        pi += 1.0 / (i * 4.0 + 1.0);
        pi -= 1.0 / (i * 4.0 + 3.0);
    }
    double *res = (double *) malloc(sizeof(double));
    *res = pi;
    pthread_exit(res);
}

int main(int argc, char *argv[]) {
    int count_threads = atoi(argv[1]);
    pthread_t threads[count_threads];

    initial_conditions *thread_context = (initial_conditions *) malloc(
            count_threads * sizeof(initial_conditions));
    int begin = 0;
    for (int i = 0; i < count_threads; ++i) {
        thread_context[i].begin_iter_idx = begin;
        thread_context[i].count_iterations = num_steps / count_threads;
        if (i == count_threads - 1) {
            thread_context[i].count_iterations += num_steps % count_threads;
        }
        begin += num_steps / count_threads;
    }

    for (int i = 0; i < count_threads; ++i) {
        pthread_create(&threads[i], NULL, thread_task,
                       &thread_context[i]);
    }

    double pi = 0;
    for (int i = 0; i < count_threads; ++i) {
        void *thread_res;
        pthread_join(threads[i], &thread_res);
        pi += *((double *) thread_res);
        free((double *)thread_res);
    }
    pi *= 4;
    printf("pi = %.15g\n", pi);
    free(thread_context);
}
