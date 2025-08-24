/*********************************************************************
Program : miniShell Version : 1.3
--------------------------------------------------------------------
skeleton code for linix/unix/minix command line interpreter
--------------------------------------------------------------------
File : minishell.c
Compiler/System : gcc/linux
********************************************************************/
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define NV 20  /* max number of command tokens */
#define NL 100 /* input buffer size */
char line[NL]; /* command input buffer */
/*
shell prompt
*/
void prompt(void) {
  // ## REMOVE THIS 'fprintf' STATEMENT BEFORE SUBMISSION
  fprintf(stdout, "\n msh> ");
  fflush(stdout);
}
/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[]) {
  pid_t frkRtnVal;  // value returned by fork sys call (this is an actual id :P)
  // int frkRtnVal;       /* value returned by fork sys call */
  char *v[NV];         /* array of pointers to command line tokens
                        */
  char *sep = " \t\n"; /* command line token separators */
  int i;               /* parse index */
  /* prompt for and process one command line at a time */
  while (1) { /* do Forever */
    prompt();

    if (fgets(line, NL, stdin) == NULL) {
      // EOF or read error
      if (feof(stdin)) {
        exit(0);
      } else {
        perror("fgets");  // this will be helpful if input fails
        exit(1);
      }
    }
    // fgets(line, NL, stdin);
    fflush(stdin);
    // This if() required for gradescope  //DOUBLE CHECK THIS
    // if (feof(stdin)) { /* non-zero on EOF */
    //  exit(0);
    //}
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000') {
      continue; /* to prompt */
    }

    // tokenize
    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL) {
        break;
      }
    }

    /*v is now a NULL terminated argv*/
    // cd built in
    if (v[0] && strcmp(v[0], "cd") == 0) {
      const char *target = v[1];

      if (target == NULL) {
        // handle cd with no arguments HOME
        target = getenv("HOME");
        if (target == NULL) {
          // if no home set then POSIX allow this to fail
          fprintf(stderr, "cd: HOME not set\n");
          continue;
        }
      }
      if (chdir(target) == -1) {
        perror("chdir");
      }
      continue;
    }

    // external command path
    frkRtnVal = fork();
    if (frkRtnVal < 0) {
      // error in parent
      perror("fork");
      continue;
    }

    if (frkRtnVal == 0) {
      // child
      execvp(v[0], v);
      // only reach here if exec failed
      perror("execvp");
      _exit(127);  // exit child DONT continue into parent code
    } else {
      // parent wait for foreground child
      if (waitpid(frkRtnVal, NULL, 0) == -1) {
        perror("waitpid");
      }
      // REMOVE PRINTF STATEMENT BEFORE SUBMISSION
      printf("%s done\n", v[0]);
    }

    //////////////OG CODE SEGMENT////////////////////////////

    /* assert i is number of tokens + 1 */
    /* fork a child process to exec the command in v[0] */
    // switch (frkRtnVal = fork()) {
    //   case -1: /* fork returns error to parent process */
    //   {
    //     break;
    //   }
    //   case 0: /* code executed only by child process */
    //   {
    //     execvp(v[0], v);
    //   }
    //   default: /* code executed only by parent process */
    //   {
    //     wait(0);
    //  REMOVE PRINTF STATEMENT BEFORE SUBMISSION
    //    printf("%s done \n", v[0]);
    //    break;
    //  }
    //} /* switch */
    ///////////////////////////////////////////////////////////////
  } /* while */
} /* main */