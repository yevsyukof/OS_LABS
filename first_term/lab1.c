#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdlib.h>
#include <stdio.h>

extern char **environ;

int main(int argc, char **argv) {
    int curOpt;
    char *options = "ispuU:cC:dvV:";
    struct rlimit rlp;
    /*
     * структура с двумя полями soft limit == cur и hard limit == max
     * используется для "измерени ресурсов" определена в resource.h и дооп types.h
     */
     char **curEnvVars;
    if (argc < 2) {
        printf("haven't any args!\n");
        return 0;
    }
    while ((curOpt = getopt(argc, argv, options)) != EOF) {
        switch (curOpt) {
            case 'i':
                printf("uid = %u\n", getuid());
                printf("euid = %u\n", geteuid());
                printf("gid = %u\n", getgid());
                printf("egid = %u\n", getegid());
                break;
            case 's':
                setpgrp(); // == setpgid(pid=0,gpid=0),если pid = 0, то исп pid вызыв проц
                break;
            case 'p':
                printf("pid = %u\n", getpid());
                printf("parent pid = %u\n", getppid());
                printf("group pid = %u\n", getpgid(0));
                break;
            case 'u':
                printf("ulimit_maxFileSize = %ld\n", ulimit(UL_GETFSIZE));
                printf("ulimit_maxMemAllocEdge = %llu\n", ulimit(UL_GMEMLIM));
                printf("ulimit_cntOfMaxOpenFiles = %llu\n", ulimit(UL_GDESLIM));
                break;
            case 'U':
                if (ulimit(UL_SETFSIZE, atol(optarg)) == -1) {
                    perror("Only super-user can increase ulimit, you are not\n");
                }
                break;
            case 'c':
                getrlimit(RLIMIT_CORE, &rlp);
                printf("core size = %lu\n", rlp.rlim_cur);
                break;
            case 'C':
                getrlimit(RLIMIT_CORE, &rlp);
                rlp.rlim_cur = atol(optarg);
                if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("You must be super-user to change core dump file size\n");
                }
                break;
            case 'd':
                printf("current working directory is: %s\n", getcwd(NULL, 100));
                break;
            case 'v':
                printf("environment variables are:\n");
                for (curEnvVars = environ; *curEnvVars; curEnvVars++) {
                    printf("%s\n", *curEnvVars);
                }
                break;
            case 'V':
                putenv(optarg);
                break;
//            case '?':
//                printf("invalid option is %c\n", optopt);
//                break; //pohody getopt sam viveded
        }
    }
}

