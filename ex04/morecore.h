#ifndef MORECORE_H
#define MORECORE_H

#define CORE_LIMIT (64 * 1024 * 1024) /* OSに要求するバイト数の上限 */

#define CORE_UNIT (64 * 1024) /* OSに要求する単位 */

extern void *morecore(int nbytes, int *realbytes);
extern int get_totalcore(void);   /* OSから得て持っている合計バイト数を返す */
extern void set_totalcore(int);    /* OSにメモリを返却した場合，持っている合計バイト数を修正する */

#endif
