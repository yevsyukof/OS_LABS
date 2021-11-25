#include <stdio.h>
#include <pthread.h>
#include <limits.h>

void *thread_task(void *arg) {
    char **str_arr = (char **)arg;

    while(*str_arr != NULL) {
        printf("%s\n", *str_arr);
        str_arr++;
    }

    pthread_exit(0);
}

int main() {
    char *str_arr1[] = {"thread_1_1", NULL};
    char *str_arr2[] = {"thread_2_1", "thread_2_2", NULL};
    char *str_arr3[] = {"thread_3_1", "thread_3_2", "thread_3_3", NULL};
    char *str_arr4[] = {"thread_4_1", "thread_4_2", "thread_4_3", "thread_4_4", NULL};
    // need to be careful with passing pointers to local variables
    pthread_t childThreadID;

    pthread_create(&childThreadID, NULL, thread_task, str_arr1);
    pthread_create(&childThreadID, NULL, thread_task, str_arr2);
    pthread_create(&childThreadID, NULL, thread_task, str_arr3);
    pthread_create(&childThreadID, NULL, thread_task, str_arr4);

    pthread_exit(0);
    printf("%d", PTHREAD_STACK_MIN);
}
