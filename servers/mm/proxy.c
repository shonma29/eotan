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
#include <errno.h>
#include <fcntl.h>
#include <services.h>
#include <string.h>
#include <fs/vfs.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "proxy.h"

#define MYNAME "mm"

typedef struct {
	unsigned short tsize;
	unsigned short rtype;
	unsigned short rsize;
} fspacket_definition_t;

static fspacket_definition_t packet_def[] = {
	{ MESSAGE_SIZE(Tattach), Rattach, MESSAGE_SIZE(Rattach) },
	{ MESSAGE_SIZE(Twalk), Rwalk, MESSAGE_SIZE(Rwalk) },
	{ MESSAGE_SIZE(Topen), Ropen, MESSAGE_SIZE(Ropen) },
	{ MESSAGE_SIZE(Tcreate), Rcreate, MESSAGE_SIZE(Rcreate) },
	{ MESSAGE_SIZE(Tread), Rread, MESSAGE_SIZE(Rread) },
	{ MESSAGE_SIZE(Twrite), Rwrite, MESSAGE_SIZE(Rwrite) },
	{ MESSAGE_SIZE(Tclunk), Rclunk, MESSAGE_SIZE(Rclunk) },
	{ MESSAGE_SIZE(Tremove), Rremove, MESSAGE_SIZE(Rremove) },
	{ MESSAGE_SIZE(Tstat), Rstat, MESSAGE_SIZE(Rstat) },
	{ MESSAGE_SIZE(Twstat), Rwstat, MESSAGE_SIZE(Rwstat) }
};

static int _walk_child(mm_file_t *, mm_process_t *, const char *,
		mm_request_t *);
static int call_device(const mm_file_t *, mm_request_t *);
static int create_tag(const mm_request_t *);
static int compact_path(char *);


int mm_attach(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_session_t *session = session_create();
		if (!session) {
			reply->data[0] = ENOMEM;
			break;
		}

		mm_file_t *file = session_create_file(session);
		if (!file) {
			session_destroy(session);
			reply->data[0] = ENOMEM;
			break;
		}

		fsmsg_t *message = &(req->message);
		message->header.type = Tattach;
		message->header.token = create_token(th->node.key, session);
		message->Tattach.tag = create_tag(req);
		message->Tattach.fid = file->node.key;
		message->Tattach.afid = NOFID;
		message->Tattach.uname = (char *) (process->uid);
		message->Tattach.aname = (char *) "/";

		file->server_id = PORT_FS;
		int result = call_device(file, req);
		//log_info("mm: attach[sid=%d fid=%d] %d\n", session->node.key,
		//		file->node.key, result);

		if (result) {
			log_err("mm: attach(%x) %d\n", file->node.key, result);
			session_destroy_file(session, file);
			session_destroy(session);
			reply->data[0] = result;
			break;
		}

		file->f_flag = O_ACCMODE;
		file->f_count = 1;
		file->f_offset = 0;
		process->wd = file;
		process->session = session;

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_open(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_descriptor_t *desc = process_create_desc(process);
		if (!desc) {
			reply->data[0] = ENOMEM;
			break;
		}

		int fd = desc->node.key;
		mm_file_t *file;
		int result = _walk(&file, process, th->node.key,
				(char *) (req->args.arg1), req,
				req->walkpath);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		desc->file = file;

		int token = create_token(th->node.key, process->session);
		int oflag = req->args.arg2;
		result = _open(file, token, oflag, req);
//		log_info("proxy: %d open[%d:%d] %d\n",
//				process->node.key, fd, file->node.key, result);
		if (result) {
			desc->file = NULL;
			process_destroy_desc(process, fd);

			int error_no = _clunk(process->session, file, token,
					req);
			if (error_no) {
				//TODO what to do?
			}

			reply->data[0] = result;
			break;
		}

		file->f_flag = oflag;
		//TODO set at last if append mode
		file->f_offset = 0;

		reply->result = fd;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_create(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		ER_UINT len = kcall->region_copy(th->node.key,
				(char*)(req->args.arg1), PATH_MAX,
				req->pathbuf);
		if (len < 0) {
			reply->data[0] = EFAULT;
			break;
		}

		if (len >= PATH_MAX) {
			reply->data[0] = ENAMETOOLONG;
			break;
		}

		mm_descriptor_t *desc = process_create_desc(process);
		if (!desc) {
			reply->data[0] = ENOMEM;
			break;
		}

		int fd = desc->node.key;
		char *parent_path = "";
		char *head = vfs_split_path(req->pathbuf, &parent_path);
		mm_file_t *file;
		fsmsg_t *message = &(req->message);
		int result = _walk(&file, process, kcall->thread_get_id(),
				parent_path, req, req->walkpath);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		int oflag = req->args.arg2;
		int token = create_token(kcall->thread_get_id(),
				process->session);
		if (_walk_child(file, process, head, req)) {
			int fid = file->node.key;
			message->header.type = Tcreate;
			message->header.token = token;
			message->Tcreate.tag = create_tag(req);
			message->Tcreate.fid = fid;
			message->Tcreate.name = head;
			message->Tcreate.perm = req->args.arg3;
			message->Tcreate.mode = oflag;
			result = call_device(file, req);
//			log_info("proxy: %d create[%d:%d] %d\n",
//					process->node.key, fd, fid, result);

			if (result) {
				//TODO test
				if (!_walk_child(file, process, head, req))
					result = _open(file, token,
							oflag | O_TRUNC, req);
			}
		} else
			result = _open(file, token, oflag | O_TRUNC, req);

		if (result) {
			//TODO test
			process_destroy_desc(process, fd);

			int error_no = _clunk(process->session, file, token,
					req);
			if (error_no) {
				//TODO what to do?
			}

			reply->data[0] = result;
			break;
		}

		desc->file = file;
		//TODO really?
		file->f_flag = oflag & O_ACCMODE;
		file->f_count = 1;
		//TODO set at last if append mode
		file->f_offset = 0;

		reply->result = fd;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_read(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_descriptor_t *desc = process_find_desc(process,
				req->args.arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		mm_file_t *file = desc->file;
		fsmsg_t *message = &(req->message);
		int result = _read(file,
				create_token(th->node.key, process->session),
				file->f_offset, req->args.arg3,
				(char*)(req->args.arg2), req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		file->f_offset += message->Rread.count;

		reply->result = message->Rread.count;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_write(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_descriptor_t *desc = process_find_desc(process,
				req->args.arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		mm_file_t *file = desc->file;
		fsmsg_t *message = &(req->message);
		message->header.type = Twrite;
		message->header.token = create_token(th->node.key,
				process->session);
		message->Twrite.tag = create_tag(req);
		message->Twrite.fid = file->node.key;
		message->Twrite.offset = file->f_offset;
		message->Twrite.count = req->args.arg3;
		message->Twrite.data = (char*)(req->args.arg2);

		int result = call_device(file, req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		file->f_offset += message->Rwrite.count;

		reply->result = message->Rwrite.count;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_close(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		int fd = req->args.arg1;
		mm_descriptor_t *desc = process_find_desc(process, fd);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		mm_file_t *file = desc->file;
		desc->file = NULL;
		if (process_destroy_desc(process, fd)) {
			//TODO what to do?
		}

		int result = _clunk(process->session, file,
				create_token(th->node.key, process->session),
				req);
		if (result) {
			log_err("proxy: %d close[%d:] err %d\n",
					process->node.key, fd, result);
			reply->data[0] = result;
			break;
		}

//		log_info("proxy: %d close[%d:] ok\n",
//				process->node.key, fd);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_remove(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_file_t *file;
		fsmsg_t *message = &(req->message);
		int result = _walk(&file, process, th->node.key,
				(char*)(req->args.arg1), req,
				req->walkpath);
		if (result) {
			reply->data[0] = result;
			break;
		}

		message->header.type = Tremove;
		message->header.token = create_token(th->node.key,
				process->session);
		message->Tremove.tag = create_tag(req);
		message->Tremove.fid = file->node.key;
		result = call_device(file, req);

//		log_info("proxy: %d remove[:%d] %d\n",
//				process->node.key, file->node.key, result);

		if (session_destroy_file(process->session, file)) {
			//TODO what to do?
		}

		if (result) {
			reply->data[0] = result;
			break;
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_stat(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_file_t *file;
		int result = _walk(&file, process, th->node.key,
				(char*)(req->args.arg1), req,
				req->walkpath);
		if (result) {
			reply->data[0] = result;
			break;
		}

		int token = create_token(th->node.key, process->session);
		result = _fstat((struct stat *) req->args.arg2, file,
				token, req);
		int error_no = _clunk(process->session, file, token, req);
		if (error_no) {
			//TODO what to do?
		}

		if (result) {
			reply->data[0] = result;
			break;
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_fstat(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_descriptor_t *desc = process_find_desc(process,
				req->args.arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		int result = _fstat((struct stat *) req->args.arg2, desc->file,
				create_token(th->node.key, process->session),
				req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_chmod(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_file_t *file;
		fsmsg_t *message = &(req->message);
		int result = _walk(&file, process, th->node.key,
				(char*)(req->args.arg1), req,
				req->walkpath);
		if (result) {
			reply->data[0] = result;
			break;
		}

		struct stat st;
		st.st_mode = req->args.arg2;
		st.st_gid = (gid_t) (-1);
		st.st_size = -1;
		st.st_mtime = -1;

		int token = create_token(th->node.key, process->session);
		message->header.type = Twstat;
		message->header.token = token;
		message->Twstat.tag = create_tag(req);
		message->Twstat.fid = file->node.key;
		message->Twstat.stat = &st;
		result = call_device(file, req);

		int error_no = _clunk(process->session, file, token, req);
		if (error_no) {
			//TODO what to do?
		}

		if (result) {
			reply->data[0] = result;
			break;
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int _walk(mm_file_t **file, mm_process_t *process, const int thread_id,
		const char *path, mm_request_t *req, char *buf)
{
	if (!(process->wd))
		//TODO what to do?
		return ECONNREFUSED;

	//TODO omit copy
	ER_UINT len = kcall->region_copy(thread_id, path, PATH_MAX, buf);
	if (len < 0)
		return EFAULT;

	if (len >= PATH_MAX)
		return ENAMETOOLONG;

	len = compact_path(buf);
	if (len < 0)
		return ENOENT;

	mm_file_t *f = session_create_file(process->session);
	if (!f)
		return ENOMEM;

	fsmsg_t *message = &(req->message);
	message->header.type = Twalk;
	message->header.token =
			create_token(kcall->thread_get_id(), process->session);
	message->Twalk.tag = create_tag(req);
	message->Twalk.fid = process->wd->node.key;
	message->Twalk.newfid = f->node.key;
	message->Twalk.nwname = len;
	message->Twalk.wname = buf;
//	log_info(MYNAME ": walk %d %d [%s] %d\n", process->session->node.key,
//			message->Twalk.fid, message->Twalk.wname,
//			message->Twalk.newfid);

	int result = call_device(process->wd, req);
	if (result)
		session_destroy_file(process->session, f);
	else {
		*file = f;
		f->server_id = process->wd->server_id;
		f->f_flag = O_ACCMODE;
		f->f_count = 1;
		f->f_offset = 0;
	}

	return result;
}

static int _walk_child(mm_file_t *file, mm_process_t *process,
		const char *name, mm_request_t *req)
{
	if (!(process->wd))
		//TODO test
		//TODO what to do?
		return ECONNREFUSED;

	size_t len = strlen(name);
	if (len >= PATH_MAX)
		//TODO test
		return ENAMETOOLONG;

	fsmsg_t *message = &(req->message);
	message->header.type = Twalk;
	message->header.token =
			create_token(kcall->thread_get_id(), process->session);
	message->Twalk.tag = create_tag(req);
	message->Twalk.fid = file->node.key;
	message->Twalk.newfid = file->node.key;
	message->Twalk.nwname = len;
	message->Twalk.wname = (char *) name;
	return call_device(file, req);
}

int _open(const mm_file_t *file, const int token, const int mode,
		mm_request_t *req)
{
	fsmsg_t *message = &(req->message);
	message->header.type = Topen;
	message->header.token = token;
	message->Topen.tag = create_tag(req);
	message->Topen.fid = file->node.key;
	message->Topen.mode = mode;
	return call_device(file, req);
}

int _read(const mm_file_t *file, const int token, const off_t offset,
		const size_t count, char *data, mm_request_t *req)
{
	fsmsg_t *message = &(req->message);
	message->header.type = Tread;
	message->header.token = token;
	message->Tread.tag = create_tag(req);
	message->Tread.fid = file->node.key;
	message->Tread.offset = offset;
	message->Tread.count = count;
	message->Tread.data = data;
	return call_device(file, req);
}

int _clunk(mm_session_t *session, mm_file_t *file, const int token,
		mm_request_t *req)
{
	file->f_count--;
	if (file->f_count > 0) {
//		log_info("proxy: close[:%d] skip\n", file->node.key);
		return 0;
	}

	fsmsg_t *message = &(req->message);
	message->header.type = Tclunk;
	message->header.token = token;
	message->Tclunk.tag = create_tag(req);
	message->Tclunk.fid = file->node.key;

	int result = call_device(file, req);

	if (session_destroy_file(session, file)) {
		//TODO what to do?
	}

	return result;
}

int _fstat(struct stat *st, const mm_file_t *file, const int token,
		mm_request_t *req)
{
	fsmsg_t *message = &(req->message);
	message->header.type = Tstat;
	message->header.token = token;
	message->Tstat.tag = create_tag(req);
	message->Tstat.fid = file->node.key;
	message->Tstat.stat = st;
	return call_device(file, req);
}

static int call_device(const mm_file_t *file, mm_request_t *req)
{
	fsmsg_t *message = &(req->message);
	message->header.ident = IDENT;
	fspacket_definition_t *def = &(packet_def[message->header.type]);
	ER_UINT size = kcall->ipc_send(file->server_id, message, def->tsize);
	if (size)
		return ECONNREFUSED;

	req->callee = file->server_id;
	fiber_switch(&(req->fiber_sp), req->receiver_sp);

	size = req->size;
	if (size >= MIN_MESSAGE_SIZE) {
		if (message->header.type == def->rtype) {
			if (size == def->rsize)
				return 0;
		} else if (message->header.type == Rerror) {
			if (size == MESSAGE_SIZE(Rerror))
				return message->Rerror.ename;
		}
	}

	return ECONNREFUSED;
}

static int create_tag(const mm_request_t *req)
{
	return req->node.key;
}

static int compact_path(char *src)
{
	char *r = src;
	if (*r == '/')
		r++;

	char *w = r;
	for (;; r++) {
		if (*r == '/')
			return (-1);

		bool last = false;
		char *word = r;
		for (;; w++, r++) {
			if (!*r) {
				last = true;
				break;
			}

			if (*r == '/')
				break;

			*w = *r;
		}

		if (((size_t) r - (size_t) word == 1)
				&& (*word == '.')) {
			w--;

			if (!last)
				continue;
		}

		if (last)
			break;

		*w = '/';
		w++;
	}

	if (((size_t) w - (size_t) src > 1)
			&& (w[-1] == '/'))
		w--;

	*w = '\0';
	return ((size_t) w - (size_t) src);
}
