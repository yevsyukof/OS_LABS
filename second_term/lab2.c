#include <stdio.h>
#include <pthread.h>

void *thread_task(void *arg) {
    char *str = "child thread\n";
    for (int i = 0; i < 10; ++i) {
        printf("%s", str);
    }
    pthread_exit(0);
}

int main() {
    pthread_t childThreadID;
    pthread_create(&childThreadID, NULL, thread_task, NULL);

    pthread_join(childThreadID, NULL);

    char *str = "parent thread\n";
    for (int i = 0; i < 10; ++i) {
        printf("%s", str);
    }

    pthread_exit(0);
}
