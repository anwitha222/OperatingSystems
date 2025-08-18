#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sighup(int signal) {
  (void)signal;
  printf("Ouch!\n");
  fflush(stdout);
}

void handle_sigint(int signal) {
  (void)signal;
  printf("Yeah!\n");
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    // check if n is positive int more than 2
    fprintf(stderr, "n not valid\n");
    return 1;
  }

  char *end = NULL;
  long n = strtol(argv[1], &end, 10);
  // check if n is not there
  if (*end != '\0' || n < 0) {
    fprintf(stderr, "n not valid\n");
    return 1;
  }

  // handle signals
  signal(SIGHUP, handle_sighup);
  signal(SIGINT, handle_sigint);

  // go through 0 to (2*n - 1)
  for (long i = 0; i < 2 * n; i++) {
    // if i is even
    if (i % 2 == 0) {
      printf("%ld\n", i);
      fflush(stdout);
      sleep(5);
    }
  }

  return 0;
}
