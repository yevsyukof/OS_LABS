#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>

int main() {
  srand(time(NULL));
  FILE *pipe[2];

  int numBuff, i, j;

  if (p2open("sort -n", pipe) == -1) {
    perror("sort -n");
    exit(1);
  }

  for (i = 0; i < 100; i++) {
      fprintf(pipe[0], "%d\n", rand() % 100);
  }
  fclose(pipe[0]);

  for (i = 0; i < 10; i++) {
    for (j = 0; j < 10; j++) {
      fscanf(pipe[1], "%d", &numBuff);
      printf("%d ", numBuff);
    }
    printf("\n");
  }

  if (p2close(pipe) == -1) {
    perror("Error closing pipe");
    exit(2);
  }
}

