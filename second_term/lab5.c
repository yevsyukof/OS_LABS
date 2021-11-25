#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void last_words(void *arg) {
    printf("%s\n", (char *)arg);
}

void *thread_task(void *arg) {
    pthread_cleanup_push(last_words, "papa, za chto????")

    while(1) {
        printf("strokaDADADADADADADADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        pthread_testcancel();
    }

    pthread_cleanup_pop(1);

    pthread_exit(0);
}

int main() {
    pthread_t childThreadID;
    pthread_create(&childThreadID, NULL, thread_task, NULL);

    sleep(2);

    pthread_cancel(childThreadID);

    pthread_exit(0); /// айайай
}
