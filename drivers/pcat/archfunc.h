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

/* timer.c */
extern void start_interval(void);
extern void intr_interval(void);
extern void timer_initialize(void);
extern void set_timer(W time, void (*func) (VP), VP argp);
extern ER unset_timer(void (*func) (VP), VP arg);
extern void check_timer(void);
extern W left_time(void (*func) (VP), VP arg);

/* rtc.c */
extern int rtc_get_time(void);

#endif /* __PCAT_ARCH_H__ */
