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

//TODO ENDIAN
#define IDENT 0x5039

#define NOFID (0)

enum fs_message_type {
	Tversion = 0x0a,
	Rversion = 0xff0a,
	Tauth = 0x0b,
	Rauth = 0xff0b,
	Tattach = 0x00,
	Rattach = 0xff00,
	Rerror = 0xff0d,
	Tflush = 0x0c,
	Rflush = 0xff0c,
	Twalk = 0x01,
	Rwalk = 0xff01,
	Topen = 0x02,
	Ropen = 0xff02,
	Tcreate = 0x03,
	Rcreate = 0xff03,
	Tread = 0x04,
	Rread = 0xff04,
	Twrite = 0x05,
	Rwrite = 0xff05,
	Tclunk = 0x06,
	Rclunk = 0xff06,
	Tremove = 0x07,
	Rremove = 0xff07,
	Tstat = 0x08,
	Rstat = 0xff08,
	Twstat = 0x09,
	Rwstat = 0xff09
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

static inline int thread_id_of_token(const int token)
{
	return ((token >> 16) & 0xffff);
}

#endif
