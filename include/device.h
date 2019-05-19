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

#dfine NOFID (0)

enum device_operation {
	operation_version = 0xff01,
	operation_auth = 0xff02,
	operation_attach = 0xff03,
	operation_flush = 0xff04,
	operation_walk = 0xff05,
	operation_open = 0xff06,
	operation_create = 0xff07,
	operation_read = 0xff08,
	operation_write = 0xff09,
	operation_clunk = 0xff0a,
	operation_remove = 0xff0b,
	operation_stat = 0xff0c,
	operation_wstat = 0xff0d
};

typedef union {
	struct {
		enum device_operation operation;
		int tag;
		size_t msize;
		char *version;
	} Tversion;
	struct {
		ssize_t size;
		int tag;
		size_t msize;
		char *version;
	} Rversion;
	struct {
		enum device_operation operation;
		int tag;
		int afid;
		char *uname;
		char *aname;
	} Tauth;
	struct {
		ssize_t size;
		int tag;
		int aqid;
	} Rauth;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		int afid;
		char *uname;
		char *aname;
	} Tattach;
	struct {
		ssize_t size;
		int tag;
		int qid;
	} Rattach;
	struct {
		ssize_t size;
		char *ename;
	} Rerror;
	struct {
		enum device_operation operation;
		int tag;
		int oldtag;
	} Tflush;
	struct {
		ssize_t size;
		int tag;
	} Rflush;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		int newfid;
		int mwname;
		char *wname;
	} Twalk;
	struct {
		ssize_t size;
		int tag;
		int nwqid;
		char *wqid;
	} Rwalk;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		int mode;
	} Topen;
	struct {
		ssize_t size;
		int tag;
		int qid;
		int iounit;
	} Ropen;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		char *name;
		int perm;
		int mode;
	} Tcreate;
	struct {
		ssize_t size;
		int tag;
		int qid;
		int iounit;
	} Rcreate;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		off_t offset;
		size_t count;
		char *data;
	} Tread;
	struct {
		ssize_t size;
		int tag;
		ssize_t count;
	} Rread;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		off_t offset;
		size_t count;
		char *data;
	} Twrite;
	struct {
		ssize_t size;
		int tag;
		ssize_t count;
	} Rwrite;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
	} Tclunk;
	struct {
		ssize_t size;
		int tag;
	} Rclunk;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
	} Tremove;
	struct {
		ssize_t size;
		int tag;
	} Rremove;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		struct stat *stat;
	} Tstat;
	struct {
		ssize_t size;
		int tag;
	} Rstat;
	struct {
		enum device_operation operation;
		int tag;
		int fid;
		struct stat *stat;
	} Twstat;
	struct {
		ssize_t size;
		int tag;
	} Rwstat;
} devmsg_t;

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
