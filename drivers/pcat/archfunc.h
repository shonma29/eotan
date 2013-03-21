/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
#ifndef __PCAT_ARCHFUNC_H__
#define __PCAT_ARCHFUNC_H__	1

#include <itron/types.h>

/* rtc.c */
extern int rtc_get_time(void);

/* 8254.c */
extern ER pit_initialize(const UW freq);

#endif /* __PCAT_ARCH_H__ */
