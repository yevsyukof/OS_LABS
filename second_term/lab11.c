#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;

void *thread_task(void *param) {
    char *str = "child thread\n";

    pthread_mutex_lock(&m1);
    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&m2);
        pthread_mutex_unlock(&m1);

        pthread_mutex_lock(&m3);
        pthread_mutex_unlock(&m2);
        printf("%s", str);

        pthread_mutex_lock(&m1);
        pthread_mutex_unlock(&m3);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;

    pthread_mutex_lock(&m2);
    pthread_create(&thread, NULL, thread_task, NULL);
    sleep(2);

    char *str = "parent thread\n";
    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&m3);
        pthread_mutex_unlock(&m2);
        printf("%s", str);

        pthread_mutex_lock(&m1);
        pthread_mutex_unlock(&m3);
        pthread_mutex_lock(&m2);
        pthread_mutex_unlock(&m1);
    }

    pthread_join(thread, NULL);
    return (EXIT_SUCCESS);
} 
