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
#define NV 20    /* max number of command tokens */
#define NL 100   /* input buffer size */
#define MAXJ 64  // max bg jobs to track
char line[NL];   /* command input buffer */

// job table!
struct job {
  int active;
  int id;  // gives job number
  pid_t pid;
  char cmd[NL];  // commands like sleep
};

static struct job jobs[MAXJ];
static int nextJobID = 1;

/*
shell prompt
*/
static void prompt(void) {
  // ## REMOVE THIS 'fprintf' STATEMENT BEFORE SUBMISSION
  // fprintf(stdout, "\n msh> ");
  // fflush(stdout); //pretty sure next line is not necessary i just forgot to
  // remove print statement oml
  if (isatty(STDIN_FILENO)) {  // only show prompt if interactive for gradescope
    fprintf(stdout, "\n msh> ");
    fflush(stdout);
  }
}

// record a bg job
static void add_job(pid_t pid, const char *cmd) {
  for (int i = 0; i < MAXJ; i++) {
    if (!jobs[i].active) {
      jobs[i].active = 1;
      jobs[i].id = nextJobID++;
      jobs[i].pid = pid;
      strncpy(jobs[i].cmd, cmd, NL - 1);
      jobs[i].cmd[NL - 1] = '\0';
      // print pid straight away
      printf("[%d] %d\n", jobs[i].id, (int)pid);
      fflush(stdout);
      return;
    }
  }
}

// find job using pid, mark it as done, and print done line
static void finish_job(pid_t pid) {
  for (int i = 0; i < MAXJ; i++) {
    if (jobs[i].active && jobs[i].pid == pid) {
      jobs[i].active = 0;
      // match format expected
      printf("[%d]+ Done                 %s\n", jobs[i].id, jobs[i].cmd);
      fflush(stdout);
      return;
    }
  }
}

// call this often to reap finished bg children
static void reap_background(void) {
  int status;
  // pid_t p;
  //  reap all finished children
  for (;;) {
    pid_t p = waitpid(-1, &status, WNOHANG);
    if (p > 0) {
      finish_job(p);
      continue;
    } else if (p == 0) {
      // no more finished children
      break;
    } else {
      if (errno != ECHILD) perror("waitpid");
    }
  }
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
    // before showing a prompt, report any completed bg jobs
    reap_background();
    prompt();

    if (fgets(line, NL, stdin) == NULL) {
      // EOF or read error
      if (feof(stdin)) {
        exit(0);
      } else {            // DOUBLE CHECK THIS
        perror("fgets");  // this will be helpful if input fails
        exit(1);
      }
    }
    // fgets(line, NL, stdin);
    // fflush(stdin); UNDEFINED FOR INPUT STREAMS
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

    if (!v[0]) continue;

    // detect bg job
    int bg = 0;
    int last = i - 1;  // last token index (i is NULL)
    if (last >= 0 && v[last]) {
      size_t L = strlen(v[last]);

      if (L == 1 && strcmp(v[last], "&") == 0) {
        // case:... "&"
        bg = 1;
        v[last] = NULL;  // remove it so execvp doesn't see &
      } else if (L > 1 && v[last][L - 1] == '&') {
        /* case:... "word&" (no space) */
        bg = 1;
        v[last][L - 1] = '\0';                   // strip trailing &
        if (v[last][0] == '\0') v[last] = NULL;  // if it became empty, drop it
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

    // command string for job messages without ampersand
    char cmdline[NL] = {0};
    {
      size_t pos = 0;
      for (int k = 0; v[k] && pos < NL - 1; k++) {
        size_t len = strlen(v[k]);
        if (pos + len + (v[k + 1] ? 1 : 0) >= NL) break;
        memcpy(cmdline + pos, v[k], len);
        pos += len;
        if (v[k + 1]) cmdline[pos++] = ' ';
      }
      cmdline[NL - 1] = '\0';
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
      if (bg) {
        /* background: do NOT wait; just announce */
        // printf("[%d] %d\n", nextJobID++, frkRtnVal); direct printf wont work
        // just call helper
        add_job(frkRtnVal, cmdline);
        // fflush(stdout); no need to double up
        /* (optional) add_job(frkRtnVal, cmdline); if you want "Done" later */
      } else {
        /* foreground: wait */
        if (waitpid(frkRtnVal, NULL, 0) == -1) {
          perror("waitpid");
        }
      }
      // parent wait for foreground child
      // if (waitpid(frkRtnVal, NULL, 0) == -1) { //should not wait and just
      // print the pid
      //  perror("waitpid");
      //}
      // REMOVE PRINTF STATEMENT BEFORE SUBMISSION
      // printf("%s done\n", v[0]);
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