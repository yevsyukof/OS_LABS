 
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

volatile int handle_signal = 0;
volatile int all_in = 1;
pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void set_flag() {
    handle_signal = 1;
}

void signal_handler() {
    set_flag();
}

typedef struct {
    int thread_num;
    int count_threads;
    double result;
} initial_conditions;

void *thread_task(void *arg) {
    initial_conditions *initial_condition = (initial_conditions *) arg;
    initial_condition->result = 0;
    int iter_idx = initial_condition->thread_num;


    while (all_in) {
        for (int i = 0; i < 100000; ++i) {
            initial_condition->result += 1.0 / (iter_idx * 4.0 + 1.0);
            initial_condition->result -= 1.0 / (iter_idx * 4.0 + 3.0);
            iter_idx += initial_condition->count_threads;
        }
        pthread_barrier_wait(&barrier);
        // т.к. возможно еще не все потоки возможно даже зашли в тело while,
        // а в критической секции all_in уже выставили в 0

        pthread_mutex_lock(&mutex);
        {
            if (handle_signal) {
                all_in = 0;
            }
        }
        pthread_mutex_unlock(&mutex);

        pthread_barrier_wait(&barrier);
        /// потоки замирают навсегда на барьере, т.к. некоторые потоки покинули тело while уже
        /// т.е. у кого-то после барьера handle_signal = 0, а у кого-то 1
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    int count_threads = atoi(argv[1]);

    pthread_t threads[count_threads];
    initial_conditions *thread_context = (initial_conditions *) malloc(
            count_threads * sizeof(initial_conditions));

    pthread_barrier_init(&barrier, NULL, count_threads);

    for (int i = 0; i < count_threads; ++i) {
        thread_context[i].count_threads = count_threads;
        thread_context[i].thread_num = i;
    }

    for (int i = 0; i < count_threads; ++i) {
        pthread_create(&threads[i], NULL, thread_task,
                       &thread_context[i]);
    }

    double pi = 0;
    for (int i = 0; i < count_threads; ++i) {
        pthread_join(threads[i], NULL);
        pi += thread_context[i].result;
    }

    pi *= 4;
    printf("\npi = %.15g\n", pi);
    pthread_barrier_destroy(&barrier);
}
