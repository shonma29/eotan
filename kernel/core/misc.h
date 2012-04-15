/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* misc.h ---
 *
 *
 */

#ifndef __ITRON_MISC_H__
#define __ITRON_MISC_H__	1

#ifdef notdef
#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#define CUTDOWN(x,align)	(((unsigned long)x) / ((unsigned long)align) * ((unsigned long)align))
#endif

/* 2 の冪乗でしか使えないが，効率は良い */
#define ROUNDUP(x,align)	((((int)x) + (align) - 1) & ~((align) - 1))
#define CUTDOWN(x,align)	((((int)x) & ~((align) - 1)))

#define TABLE_SIZE(table)	(sizeof(table) / sizeof(table[0]))
#define ISDIGIT(x)		((x >= '0') && (x <= '9'))
#define MAX(a,b)		((a > b) ? a : b)
#define MIN(a,b)		((a < b) ? a : b)

#define PAGES(x)		(ROUNDUP(x, PAGE_SIZE) >> PAGE_SHIFT)


#define ASSERT(exp,mes)		\
if (exp) \
{\
   printk ("ASSERT (%s %d): %s\n", __FILE__, __LINE__, mes); \
}
  

/* rtov --- 実メモリアドレスを仮想メモリアドレスに変換する。
 */
#define RTOV(real)	((real)  | 0x80000000UL)
#define VTOR(vaddr)	((vaddr) & 0x7FFFFFFFL)

/* ptov --- 実メモリのページ番号から仮想メモリアドレスを割り出す。
 */
#define PTOV(page)	((page << PAGE_SHIFT) | 0x80000000UL)


#endif /* __ITRON_MISC_H__ */
