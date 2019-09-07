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
#include <syslog.h>
#include <dev/device.h>
#include <sys/time.h>

#define log_emerg(...) syslog(LOG_EMERG, __VA_ARGS__)
#define log_alert(...) syslog(LOG_ALERT, __VA_ARGS__)
#define log_crit(...) syslog(LOG_CRIT, __VA_ARGS__)
#define log_err(...) syslog(LOG_ERR, __VA_ARGS__)
#define log_warning(...) syslog(LOG_WARNING, __VA_ARGS__)
#define log_notice(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define log_info(...) syslog(LOG_INFO, __VA_ARGS__)

#ifdef DEBUG
#define log_debug(...) syslog(LOG_DEBUG, __VA_ARGS__)
#else
#define log_debug(...)
#endif

extern void time_get_raw(struct timespec *);
extern ER time_get(SYSTIM *);
extern ER time_set(SYSTIM *);

extern ER define_handler(INHNO, T_DINH *);
extern ER enable_interrupt(INHNO);

extern vdriver_t *device_find(const int);

#endif
