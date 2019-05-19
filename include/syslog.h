#ifndef __SYSLOG_H__
#define __SYSLOG_H__ 1
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
#include <device.h>
#include <stddef.h>
#include <sys/types.h>

//#define SYSLOG_MAX_LENGTH (1024)
#define SYSLOG_MAX_LENGTH (80)

enum syslog_channel {
	channel_kernlog,
	channel_syslog
};

enum log_level {
	LOG_EMERG,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
};

typedef union {
	struct {
		enum device_operation operation;
		enum syslog_channel fid;
		size_t count;
	} Tread;
	struct {
		ssize_t count;
		char data[SYSLOG_MAX_LENGTH];
	} Rread;
	struct {
		enum device_operation operation;
		enum log_level priority;
		size_t count;
		char data[SYSLOG_MAX_LENGTH];
	} Twrite;
	struct {
		ssize_t count;
	} Rwrite;
} syslog_t;

extern void syslog(const int,  const char *, ...);

#endif
