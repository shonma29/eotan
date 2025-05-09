#ifndef _LIBSERV_H_
#define _LIBSERV_H_
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <core.h>
#include <interrupt.h>
#include <syslog.h>
#include <dev/device.h>
#include <nerve/kcall.h>
#include <sys/time.h>

#ifndef log_emerg
#define log_emerg(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_alert
#define log_alert(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_crit
#define log_crit(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_err
#define log_err(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_warning
#define log_warning(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_notice
#define log_notice(...) kcall->printk(__VA_ARGS__)
#endif
#ifndef log_info
#define log_info(...) kcall->printk(__VA_ARGS__)
#endif

#ifndef log_debug
#ifdef DEBUG
#define log_debug(...) kcall->printk(__VA_ARGS__)
#else
#define log_debug(...)
#endif
#endif

extern void time_get_raw(struct timespec *);
extern ER time_get(SYSTIM *);
extern ER time_set(SYSTIM *);

extern ER_ID create_isr(T_CISR *);
extern ER destroy_isr(ID);
extern ER enable_interrupt(INTNO);

extern device_info_t *device_find(const char *);

extern int copy_from_user(void *, void *, const size_t);
extern int copy_to_user(void *, void *, const size_t);

extern int get_path(char *, const int, const char *);

#endif
