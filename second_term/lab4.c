#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread_task(void *arg) {

    while(1) {
        printf("strokaDADADADADADADADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    }

    pthread_exit(0);
}

int main() {
    pthread_t childThreadID;
    pthread_create(&childThreadID, NULL, thread_task, NULL);

    sleep(2);

    pthread_cancel(childThreadID);
}
