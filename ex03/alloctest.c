#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include "alloc2.h"

#define NUM_MAX 100
#define SIZE (ALLOCSIZE / 2 / NUM_MAX)

#define REPEAT 100

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
// それぞれの領域のサイズはSIZEバイト(ヘッダは含まない)
// table[0]..table[current_num - 1]までが使用中
void *table[NUM_MAX];
int current_num = 0;

// table内のポインタ数をnum個に変化させる
void alloc_to(int num, int size)
{
  if (num < 0 || NUM_MAX < num) {
    error("illegal num %d\n", num);
  }
  // もし num > current_num なら増やす
  while (current_num < num) {
    table[current_num] = alloc2(size);
    if (table[current_num] == NULL) {
      error("allocation failed\n");
    }
    current_num++;
  }
  // もし num < current_num なら減らす
  while (current_num > num) {
    afree2(table[--current_num]);
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

void progress(void)
{
  putchar('.'); fflush(stdout);
}

int main(void)
{
  int pass;
  void *p;

  /* テスト1: 上限を越えた割り付けを行なうと，失敗する． */
  CATCH {
    printf("test1: "); fflush(stdout);
    alloc_to(NUM_MAX, SIZE);

    p = alloc2(ALLOCSIZE);		/* should fail */
    if (p != NULL) {
      error("over allocation\n");
    }
    printf("passed.\n");
  }

  /* テスト2: ランダムな順序で割り付けと解放を繰り返す */
  CATCH {
    printf("test2: "); fflush(stdout);
    for (pass = 0; pass < REPEAT; pass++ ) {
      alloc_to(random_number_upto(NUM_MAX), SIZE); progress();
      check_overlap(SIZE); progress();
      randomize();
    }
    printf("passed.\n");
  }
  alloc_to(0, SIZE);

  /* テスト3: ちゃんとマージされているかテスト */
  CATCH {
    printf("test3: "); fflush(stdout);
    alloc_to(1, ALLOCSIZE / 2);
    printf("passed.\n");
  }

  return 0;
}
