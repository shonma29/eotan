#ifndef _DEVICE_H_
#define _DEVICE_H_
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
#include <stddef.h>
#include <sys/stat.h>

#define DEV_BUF_SIZE (1024)

#define NOFID (0)

enum dev_message_type {
	Tversion = 0xff01,
	Rversion = 0xff02,
	Tauth = 0xff03,
	Rauth = 0xff04,
	Tattach = 0xff05,
	Rattach = 0xff06,
	Rerror = 0xff07,
	Tflush = 0xff08,
	Rflush = 0xff09,
	Twalk = 0xff0a,
	Rwalk = 0xff0b,
	Topen = 0xff0c,
	Ropen = 0xff0d,
	Tcreate = 0xff0e,
	Rcreate = 0xff0f,
	Tread = 0xff10,
	Rread = 0xff11,
	Twrite = 0xff12,
	Rwrite = 0xff13,
	Tclunk = 0xff14,
	Rclunk = 0xff15,
	Tremove = 0xff16,
	Rremove = 0xff17,
	Tstat = 0xff18,
	Rstat = 0xff19,
	Twstat = 0xff1a,
	Rwstat = 0xff1b
};

typedef struct {
	//TODO enum dev_message_type type;
	int type;
	union {
		struct {
			int tag;
			size_t msize;
			char *version;
		} Tversion;
		struct {
			int tag;
			size_t msize;
			char *version;
		} Rversion;
		struct {
			int tag;
			int afid;
			char *uname;
			char *aname;
		} Tauth;
		struct {
			int tag;
			int aqid;
		} Rauth;
		struct {
			int tag;
			int fid;
			int afid;
			char *uname;
			char *aname;
		} Tattach;
		struct {
			int tag;
			int qid;
		} Rattach;
		struct {
			int tag;
			int ename;
		} Rerror;
		struct {
			int tag;
			int oldtag;
		} Tflush;
		struct {
			int tag;
		} Rflush;
		struct {
			int tag;
			int fid;
			int newfid;
			int nwname;
			char *wname;
		} Twalk;
		struct {
			int tag;
			int nwqid;
			char *wqid;
		} Rwalk;
		struct {
			int tag;
			int fid;
			int mode;
		} Topen;
		struct {
			int tag;
			int qid;
			int iounit;
		} Ropen;
		struct {
			int tag;
			int fid;
			char *name;
			int perm;
			int mode;
		} Tcreate;
		struct {
			int tag;
			int qid;
			int iounit;
		} Rcreate;
		struct {
			int tag;
			int fid;
			off_t offset;
			size_t count;
			char *data;
		} Tread;
		struct {
			int tag;
			ssize_t count;
		} Rread;
		struct {
			int tag;
			int fid;
			off_t offset;
			size_t count;
			char *data;
		} Twrite;
		struct {
			int tag;
			ssize_t count;
		} Rwrite;
		struct {
			int tag;
			int fid;
		} Tclunk;
		struct {
			int tag;
		} Rclunk;
		struct {
			int tag;
			int fid;
		} Tremove;
		struct {
			int tag;
		} Rremove;
		struct {
			int tag;
			int fid;
			struct stat *stat;
		} Tstat;
		struct {
			int tag;
		} Rstat;
		struct {
			int tag;
			int fid;
			struct stat *stat;
		} Twstat;
		struct {
			int tag;
		} Rwstat;
	};
} devmsg_t;

#define MESSAGE_SIZE(t) \
	(sizeof(((devmsg_t*)0)->type) + sizeof(((devmsg_t*)0)->t))
#define MIN_MESSAGE_SIZE (sizeof(((devmsg_t*)0)->type) + sizeof(int))

//TODO move to other header
typedef struct _vdriver_t {
	unsigned int id;
	unsigned char *name;
	const size_t size;
	int (*detach)(void);
	int (*open)(void);
	int (*close)(const int);
	int (*read)(char *, const int, const off_t, const size_t);
	int (*write)(char *, const int, const off_t, const size_t);
} vdriver_t;

#endif
