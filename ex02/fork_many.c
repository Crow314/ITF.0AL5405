#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void child(int r) {
  printf("I'm child %d, sleeping %d seconds.\n", getpid(), r);
  sleep(r);  // r秒間待つ。
}

int parent(int);

int main(int argc, char *argv[]) {
  if (argc != 2) return 1;
  int n = atoi(argv[1]);
  int children[n];

  // n個の子プロセスをforkする
  for (int i = 0; i < n; i++) {
    int r = i % 5;
    int pid = fork();
    if (pid < 0) {
      fprintf(stderr, "%d: can't fork.\n", getpid());
      perror("fork");
      return 1;
    } else if (pid == 0) {
      // 子プロセスとして動作し，main()を終了する．
      child(r);
      return r;
    } else {
      // 子プロセスのpidを記録する．
      children[i] = pid;
    }
  }

  // 子プロセスの約半数をランダムにシグナルで中断する．
  for (int i = 0; i < n; i++) {
    if (random() % 10 > 5) {
      kill(children[i], SIGINT);
    }
  }
  
  // 子プロセスの状態を表示する．
  return parent(n);
}


int parent(int n) {
    // ... （ここを埋める） ...
  return 0;
}
