#ifndef _FS_PROTOCOL_H_
#define _FS_PROTOCOL_H_
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

#define IDENT 0x5039

#define NOFID (0)

enum fs_message_type {
	Tversion = 0xff01,
	Rversion = 0xff02,
	Tauth = 0xff03,
	Rauth = 0xff04,
	Tattach = 0,
	Rattach = 0xff06,
	Rerror = 0xff07,
	Tflush = 0xff08,
	Rflush = 0xff09,
	Twalk = 1,
	Rwalk = 0xff0b,
	Topen = 2,
	Ropen = 0xff0d,
	Tcreate = 3,
	Rcreate = 0xff0f,
	Tread = 4,
	Rread = 0xff11,
	Twrite = 5,
	Rwrite = 0xff13,
	Tclunk = 6,
	Rclunk = 0xff15,
	Tremove = 7,
	Rremove = 0xff17,
	Tstat = 8,
	Rstat = 0xff19,
	Twstat = 9,
	Rwstat = 0xff1b
};

struct _Tversion {
	int tag;
	size_t msize;
	char *version;
};

struct _Rversion {
	int tag;
	size_t msize;
	char *version;
};

struct _Tauth {
	int tag;
	int afid;
	char *uname;
	char *aname;
};

struct _Rauth {
	int tag;
	int aqid;
};

struct _Tattach {
	int tag;
	int fid;
	int afid;
	char *uname;
	char *aname;
};

struct _Rattach {
	int tag;
	int qid;
};

struct _Rerror {
	int tag;
	int ename;
};

struct _Tflush {
	int tag;
	int oldtag;
};

struct _Rflush {
	int tag;
};

struct _Twalk {
	int tag;
	int fid;
	int newfid;
	int nwname;
	char *wname;
};

struct _Rwalk {
	int tag;
	int nwqid;
	char *wqid;
};

struct _Topen {
	int tag;
	int fid;
	int mode;
};

struct _Ropen {
	int tag;
	int qid;
	int iounit;
};

struct _Tcreate {
	int tag;
	int fid;
	char *name;
	int perm;
	int mode;
};

struct _Rcreate {
	int tag;
	int qid;
	int iounit;
};

struct _Tread {
	int tag;
	int fid;
	off_t offset;
	size_t count;
	char *data;
};

struct _Rread {
	int tag;
	ssize_t count;
};

struct _Twrite {
	int tag;
	int fid;
	off_t offset;
	size_t count;
	char *data;
};

struct _Rwrite {
	int tag;
	ssize_t count;
};

struct _Tclunk {
	int tag;
	int fid;
};

struct _Rclunk {
	int tag;
};

struct _Tremove {
	int tag;
	int fid;
};

struct _Rremove {
	int tag;
};

struct _Tstat {
	int tag;
	int fid;
	struct stat *stat;
};

struct _Rstat {
	int tag;
};

struct _Twstat {
	int tag;
	int fid;
	struct stat *stat;
};

struct _Rwstat {
	int tag;
};

typedef struct {
	struct {
		unsigned short ident;
		unsigned short type;
		int token;
	} header;
	union {
		struct _Tversion Tversion;
		struct _Rversion Rversion;
		struct _Tauth Tauth;
		struct _Rauth Rauth;
		struct _Tattach Tattach;
		struct _Rattach Rattach;
		struct _Rerror Rerror;
		struct _Tflush Tflush;
		struct _Rflush Rflush;
		struct _Twalk Twalk;
		struct _Rwalk Rwalk;
		struct _Topen Topen;
		struct _Ropen Ropen;
		struct _Tcreate Tcreate;
		struct _Rcreate Rcreate;
		struct _Tread Tread;
		struct _Rread Rread;
		struct _Twrite Twrite;
		struct _Rwrite Rwrite;
		struct _Tclunk Tclunk;
		struct _Rclunk Rclunk;
		struct _Tremove Tremove;
		struct _Rremove Rremove;
		struct _Tstat Tstat;
		struct _Rstat Rstat;
		struct _Twstat Twstat;
		struct _Rwstat Rwstat;
	};
} fsmsg_t;

#define MESSAGE_SIZE(t) \
	(sizeof(((fsmsg_t*)0)->header) + sizeof(((fsmsg_t*)0)->t))
#define MIN_MESSAGE_SIZE (sizeof(((fsmsg_t*)0)->header) + sizeof(int))

#endif
