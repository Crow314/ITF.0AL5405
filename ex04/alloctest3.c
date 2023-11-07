#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include "morecore.h"
#include "alloc3.h"

#define NUM_MAX 10000
#define SIZE (CORE_LIMIT / 2 / NUM_MAX)

#define REPEAT 10

jmp_buf env;
#define CATCH if (setjmp(env) == 0)
#define RAISE() longjmp(env, 1)

void error(char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  RAISE();
}

// tableに，割り付け済み領域へのポインタを格納する．
// table[0]..table[current_num - 1]までが使用中
char *table[NUM_MAX];
int current_num = 0;

// table内のポインタ数をnum個に変化させる
void alloc_to(int num, int size)
{
  if (num < 0 || NUM_MAX < num) {
    error("illegal num %d\n", num);
  }
  // もし num > current_num なら増やす
  while (current_num < num) {
    table[current_num] = alloc3(size);
    if (table[current_num] == NULL) {
      error("allocation failed\n");
    }
    current_num++;
  }
  // もし num < current_num なら減らす
  while (current_num > num) {
    afree3(table[--current_num]);
  }
}

// 0..(n-1)の一様乱数を返す
int random_number_upto(int n) {
  return rand() / (1 + RAND_MAX / n);
}

// table内をランダムに並べ替える
void randomize(void) {
  int i;
  for (i = 0; i < current_num; i++) {
    int j = random_number_upto(current_num);
    void *p = table[i];
    table[i] = table[j];
    table[j] = p;
  }
}

// 割り付け済みの領域に重なりがないかチェックする
void check_overlap(int size) {
  int i, j;
  for (i = 0; i < current_num; i++) {
    for (j = 0; j < current_num; j++) {
      if (i != j) {
	int distance = table[i] - table[j];
	if (distance < 0) {
	  distance = -distance;
	}
	if (distance < size) {
	  error("overlap! distance(%d, %d) = %d\n", i, j, distance);
	}
      }
    }
  }
}

int check_writable(int size) {
  int i, j;
  // 割り付け済みの領域に書き込めるかチェックする
  for (i = 0; i < current_num; i++) {
    for (j = 0; j < size; j++) {
      table[i][j] = (char)(i + j);
    }
  }
  // 書き込んだ値が変わっていないかどうかチェックする
  for (i = 0; i < current_num; i++) {
    for (j = 0; j < size; j++) {
      if (table[i][j] != (char)(i + j)) {
	error("written value changed\n");
      }
      // 値をクリアする
      table[i][j] = 0;
    }
  }
  return 0;
}

void progress(void) {
  putchar('.'); fflush(stdout);
}

void alarm_handler(int ignore)
 {
   printf("Time out\n");
   exit(1);
 }

struct sigaction action = {
  &alarm_handler, 0, 0
};

struct itimerval t = {
  { 0, 0 },
  { 10, 0 }
};

struct itimerval t2;

int main(void)
{
  int pass;
  void *p;

  if (sigaction(SIGALRM, &action, NULL) < 0) return 1;
  if (setitimer(ITIMER_REAL, &t, NULL) < 0) return 2;

  /* テスト1: 特殊なケースで失敗しないことをチェック */
  CATCH {
    printf("test1A: "); fflush(stdout);
    alloc3(0);
    int i;
    for (i = 0; i < 10; i++) {
      int size = CORE_UNIT - i * 8;
      alloc_to(i + 2, size); progress();
      check_writable(size); progress();
      alloc_to(0, size);
    }
    printf("passed.\n");
  }
  CATCH {
    printf("test1B: "); fflush(stdout);
    int size = CORE_UNIT - 16;
    alloc_to(CORE_LIMIT / size / 3, size); progress();
    check_writable(size); progress();
    void *p = alloc3(CORE_LIMIT / 2);
    if (p == NULL) error("allocation failed\n");
    alloc_to(0, size);
    afree3(p);
    printf("passed.\n");
  }

  CATCH {
    printf("test1C: "); fflush(stdout);
    int i;
    for (i = 0; i < 10; i++) {
      int size = CORE_UNIT - (i - 5) * 8;
      alloc_to(i + 500, size); progress();
      check_writable(size); progress();
      alloc_to(0, size);
    }
    printf("passed.\n");
  }

  CATCH {
    printf("test1D: "); fflush(stdout);
    alloc_to(1, CORE_LIMIT / 2);
    check_writable(CORE_LIMIT / 2);
    progress();
    printf("passed.\n");
  }

  /* テスト2: ランダムな順序で割り付けと解放を繰り返す
            (途中で，重なりがないことをチェック) */
  CATCH {
    printf("test2: "); fflush(stdout);
    for (pass = 0; pass < REPEAT; pass++ ) {
      alloc_to(random_number_upto(NUM_MAX), SIZE); progress();
      check_overlap(SIZE); progress();
      check_writable(SIZE); progress();
      randomize();
    }
    printf("passed.\n");
  }
  alloc_to(0, SIZE);

  /* テスト3: ちゃんとマージされているかテスト */
  CATCH {
    printf("test3: "); fflush(stdout);
    alloc_to(1, CORE_LIMIT / 2);
    check_writable(CORE_LIMIT / 2);
    printf("passed.\n");
  }

  /* テスト4: 上限を越えた割り付けを行なうと，失敗する． */
  CATCH {
    printf("test4: "); fflush(stdout);
    // alloc_to(NUM_MAX, SIZE);

    alloc3(CORE_LIMIT);		/* may or may not fail */
    p = alloc3(CORE_LIMIT);		/* must fail */
    if (p != NULL) {
      error("over allocation\n");
    }
    printf("passed.\n");
  }
  
  return 0;
}
