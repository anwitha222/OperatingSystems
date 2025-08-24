/*********************************************************************
Program : miniShell Version : 1.3
--------------------------------------------------------------------
skeleton code for linux/unix/minix command line interpreter
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

#define NV 20   /* max number of command tokens */
#define NL 100  /* input buffer size */
#define MAXJ 64 /* max background jobs to track */

static char line[NL]; /* command input buffer */

// bg job table
struct job {
  int active;
  int id;  // job number
  pid_t pid;
  char cmd[NL];  // like sleep 2 etc
};

static struct job jobs[MAXJ];
static int nextJobID = 1;

// print prompt if interactive
static void prompt(void) {
  if (isatty(STDIN_FILENO)) {
    fprintf(stdout, "\n msh> ");
    fflush(stdout);
  }
}

// add new bg job, print start message
static void add_job(pid_t pid, const char *cmd) {
  for (int i = 0; i < MAXJ; i++) {
    if (!jobs[i].active) {
      jobs[i].active = 1;
      jobs[i].id = nextJobID++;
      jobs[i].pid = pid;
      strncpy(jobs[i].cmd, cmd, NL - 1);
      jobs[i].cmd[NL - 1] = '\0';
      printf("[%d] %d\n", jobs[i].id, (int)pid);
      fflush(stdout);
      return;
    }
  }
}

// when a bg child exits, print the Done line
static void finish_job(pid_t pid) {
  for (int i = 0; i < MAXJ; i++) {
    if (jobs[i].active && jobs[i].pid == pid) {
      jobs[i].active = 0;
      printf("[%d]+ Done                 %s\n", jobs[i].id, jobs[i].cmd);
      fflush(stdout);
      return;
    }
  }
}

// reap all finished background children, call this each loop
static void reap_background(void) {
  int status;
  // pid_t p;
  //  reap all finished children
  for (;;) {
    pid_t p = waitpid(-1, &status, WNOHANG);
    if (p > 0) {
      finish_job(p);
    } else if (p == 0) {
      break;  // no more finished children
    } else {
      if (errno != ECHILD) perror("waitpid");
      break;
    }
  }
}

int main(void) {
  // make output appear quickly when not interactive
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IOLBF, 0);

  /* argk - number of arguments */
  /* argv - argument vector from command line */
  /* envp - environment pointer */

  char *v[NV];  // argv-style token array
  const char *sep = " \t\n";

  while (1) {
    reap_background();  // show any Done lines from the loop prior
    prompt();

    if (fgets(line, NL, stdin) == NULL) {
      if (feof(stdin)) {
        // before exiting, reap any last finished bg jobs
        reap_background();
        exit(0);
      } else {
        perror("fgets");
        exit(1);
      }
    }

    // ignore blank lines and comments
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;

    /* tokenize */
    v[0] = strtok(line, sep);
    int i;
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL) break;
    }
    if (!v[0]) continue;  // nothing to do

    // detect background '&' (both as separate token and as a trailing
    // character)
    int bg = 0;
    int last = i - 1;  // i points to NULL, so last = i-1
    if (last >= 0 && v[last]) {
      size_t L = strlen(v[last]);
      if (L == 1 && strcmp(v[last], "&") == 0) {
        bg = 1;
        v[last] = NULL;  // strip the & token
      } else if (L > 1 && v[last][L - 1] == '&') {
        bg = 1;
        v[last][L - 1] = '\0';  // strip trailing &
        if (v[last][0] == '\0') v[last] = NULL;
      }
    }

    // built-in: cd (must run in parent)
    if (strcmp(v[0], "cd") == 0) {
      const char *target = v[1];
      if (!target) {
        target = getenv("HOME");
        if (!target) {
          fprintf(stderr, "cd: HOME not set\n");
          continue;
        }
      }
      if (chdir(target) == -1) {
        perror("chdir");
      }
      continue;
    }

    /* build a printable command line (without &) for Done message */
    char cmdline[NL] = {0};
    {
      size_t pos = 0;
      for (int k = 0; v[k] && pos < NL - 1; k++) {
        size_t len = strlen(v[k]);
        if (pos + len + (v[k + 1] ? 1 : 0) >= NL) break;
        memcpy(cmdline + pos, v[k], len);
        pos += len;
        if (v[k + 1]) cmdline[pos++] = ' ';
        // parent wait for foreground child
        // if (waitpid(frkRtnVal, NULL, 0) == -1) { //should not wait and just
        // print the pid
        //  perror("waitpid");
        //}
        // REMOVE PRINTF STATEMENT BEFORE SUBMISSION
        // printf("%s done\n", v[0]);
      }
      cmdline[NL - 1] = '\0';
    }

    /* fork & exec external command */
    pid_t child = fork();
    if (child < 0) {
      perror("fork");
      continue;
    }

    if (child == 0) {
      execvp(v[0], v);
      perror("execvp"); /* only reached on failure */
      _exit(127);
    } else {
      if (bg) {
        /* background: don't wait; just record & report */
        add_job(child, cmdline);
      } else {
        /* foreground: wait */
        if (waitpid(child, NULL, 0) == -1) {
          perror("waitpid");
        }
      }
    }
  }
}
