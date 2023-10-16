#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void child(char *command_path, char *args[]) {
  execv(command_path, args);
  // Come here only when execv() failed.
  fprintf(stderr, "execv failed.\n");
  perror(command_path);
  exit(1);
}

void parent(int pid) {
  int status;
  if ((pid = waitpid(pid, &status, 0)) < 0) {
    perror("waitpid");
  } else {
    printf("child %d has ", pid);
    if (WIFEXITED(status)) {
      printf("exited with status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("terminated by signal: %d\n", WTERMSIG(status));
    }
  }
}

void fork_and_exec_or_wait(char *command_path, char *args[]) {
  int pid = fork();
  if (pid < 0) {
    fprintf(stderr, "%d: can't fork.\n", getpid());
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    child(command_path, args);
  } else {
    parent(pid);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    // No args.
    return 0;
  }
  fork_and_exec_or_wait(argv[1], argv + 1);
  fork_and_exec_or_wait(argv[1], argv + 1);

  return 0;
}
