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
#include <mpufunc.h>
#include <fs/vfs.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include "mm.h"
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

static int call_device(const mm_file_t *, mm_request_t *);
static int create_tag(const mm_request_t *);
static long copy_path(char *, const char *, const long);
static char *omit_previous_entry(long *, char *, char *);


int _attach(mm_file_t **root, mm_request_t *req, mm_thread_t *th,
		const int server_id)
{
	mm_session_t *session = session_create(server_id);
	if (!session)
		return ENOMEM;

	mm_file_t *file = session_create_file(session);
	if (!file) {
		session_destroy(session);
		return ENOMEM;
	}

	session->server_id = server_id;

	fsmsg_t *message = &(req->message);
	message->header.type = Tattach;
	message->header.token = create_token(th->node.key, session);
	message->Tattach.tag = create_tag(req);
	message->Tattach.fid = file->node.key;
	message->Tattach.afid = NOFID;
	message->Tattach.uname = (char *) (get_process(th)->uid);
	message->Tattach.aname = (char *) PATH_ROOT;

	int result = call_device(file, req);
	//log_info("proxy: attach[sid=%x fid=%d] %d\n", session->node.key,
	//		file->node.key, result);

	if (result) {
		log_err("proxy: attach(%x) %d\n", file->node.key, result);
		session_destroy_file(file);
		return result;
	}

	file->f_offset = 0;
	file->f_flag = O_ACCMODE;
	file->f_count = 1;
	*root = file;
	return 0;
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
				(char *) (req->args.arg1), req);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		desc->file = file;

		int token = create_token(th->node.key, file->session);
		int oflag = req->args.arg2;
		result = _open(file, token, oflag, req);
//		log_info("proxy: %d open[%d:%d] %d\n",
//				process->node.key, fd, file->node.key, result);
		if (result) {
			desc->file = NULL;
			process_destroy_desc(process, fd);

			int error_no = _clunk(file, token, req);
			if (error_no) {
				//TODO what to do?
			}

			reply->data[0] = result;
			break;
		}

		//TODO set at last if append mode
		file->f_offset = 0;
		file->f_flag = oflag;

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
		int result = get_path(req->pathbuf, th->node.key,
				(char *) (req->args.arg1));
		if (result) {
			reply->data[0] = result;
			break;
		}

		ER_UINT len = calc_path(req->walkpath, process->local->wd,
				req->pathbuf, PATH_MAX);
//TODO test
		if (len == ERR_INSUFFICIENT_BUFFER)
			return ENAMETOOLONG;
//TODO test
		else if (len < 0)
			return EINVAL;

		bool in_root;
		char *name = strrchr(req->walkpath, '/');//TODO slow
		if (name == req->walkpath)
			in_root = true;
		else {
			//TODO test
			//TODO is needed?
//			if (!name)
//				return EINVAL;

			in_root = false;
			*name = '\0';
		}
		name++;

		mm_descriptor_t *desc = process_create_desc(process);
		if (!desc) {
			reply->data[0] = ENOMEM;
			break;
		}

		int fd = desc->node.key;
		mm_file_t *file;
		fsmsg_t *message = &(req->message);
		result = _walk(&file, process, 0,
				in_root ? NULL : req->walkpath, req);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		int oflag = req->args.arg2;
		int token = create_token(kcall->thread_get_id(), file->session);
		if (_walk_child(file, file, name, req)) {
			int fid = file->node.key;
			message->header.type = Tcreate;
			message->header.token = token;
			message->Tcreate.tag = create_tag(req);
			message->Tcreate.fid = fid;
			message->Tcreate.name = name;
			message->Tcreate.perm = req->args.arg3;
			message->Tcreate.mode = oflag;
			result = call_device(file, req);
//			log_info("proxy: %d create[%d:%d] %d\n",
//					process->node.key, fd, fid, result);
			if (result) {
				if (!_walk_child(file, file, name, req))
					result = _open(file, token,
							oflag | O_TRUNC, req);
			}
		} else
			result = _open(file, token, oflag | O_TRUNC, req);

		if (result) {
			//TODO test
			process_destroy_desc(process, fd);

			int error_no = _clunk(file, token, req);
			if (error_no) {
				//TODO what to do?
			}

			reply->data[0] = result;
			break;
		}

		desc->file = file;
		//TODO set at last if append mode
		file->f_offset = 0;
		//TODO really?
		file->f_flag = oflag & O_ACCMODE;
		file->f_count = 1;

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
				create_token(th->node.key, file->session),
				file->f_offset, req->args.arg3,
				(char *) (req->args.arg2), req);
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
				file->session);
		message->Twrite.tag = create_tag(req);
		message->Twrite.fid = file->node.key;
		message->Twrite.offset = file->f_offset;
		message->Twrite.count = req->args.arg3;
		message->Twrite.data = (char *) (req->args.arg2);

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

		int result = _clunk(file,
				create_token(th->node.key, file->session), req);
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
				(char *) (req->args.arg1), req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		message->header.type = Tremove;
		message->header.token = create_token(th->node.key,
				file->session);
		message->Tremove.tag = create_tag(req);
		message->Tremove.fid = file->node.key;
		result = call_device(file, req);

//		log_info("proxy: %d remove[:%d] %d\n",
//				process->node.key, file->node.key, result);

		if (session_destroy_file(file)) {
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
				(char *) (req->args.arg1), req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		int token = create_token(th->node.key, file->session);
		result = _fstat((struct stat *) req->args.arg2, file,
				token, req);
		int error_no = _clunk(file, token, req);
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
				create_token(th->node.key, desc->file->session),
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
				(char *) (req->args.arg1), req);
		if (result) {
			reply->data[0] = result;
			break;
		}

		struct stat st;
		st.st_mode = req->args.arg2;
		st.st_gid = (gid_t) (-1);
		st.st_size = -1;
		st.st_mtime = -1;

		int token = create_token(th->node.key, file->session);
		message->header.type = Twstat;
		message->header.token = token;
		message->Twstat.tag = create_tag(req);
		message->Twstat.fid = file->node.key;
		message->Twstat.stat = &st;
		result = call_device(file, req);

		int error_no = _clunk(file, token, req);
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
		const char *path, mm_request_t *req)
{
	int result;
	ER_UINT len;
	if (!path)
		len = 0;
	else if (path == req->walkpath)
		len = strlen(req->walkpath);//TODO slow
	else {
		result = get_path(req->pathbuf, thread_id, path);
		if (result)
			return result;

		len = calc_path(req->walkpath, process->local->wd, req->pathbuf,
				PATH_MAX);
//TODO test
		if (len == ERR_INSUFFICIENT_BUFFER)
			return ENAMETOOLONG;
//TODO test
		else if (len < 0)
			return EINVAL;
	}

	int offset = 0;
	mm_file_t *root = process->root;
	for (list_t *p = process->namespaces.next;
			!list_is_edge(&(process->namespaces), p); p = p->next) {
		mm_namespace_t *ns = getNamespaceFromBrothers(p);
		int i;
		for (i = 0; req->walkpath[i] == ns->name[i]; i++)
			if (!(ns->name[i]))
				break;

		if (!(ns->name[i])) {
			if (req->walkpath[i] == PATH_DELIMITER) {
				offset = i;
				len -= offset;
				root = ns->root;
				break;
			} else if (!(req->walkpath[i])) {
				req->walkpath[0] = PATH_DELIMITER;
				req->walkpath[1] = PATH_NUL;
				len = 1;
				root = ns->root;
				break;
			}
		}
	}

	if (!root)
		//TODO what to do?
		//TODO check process->local->wd_len?
		return ECONNREFUSED;

	mm_file_t *f = session_create_file(root->session);
	if (!f)
		return ENOMEM;

	fsmsg_t *message = &(req->message);
	message->header.type = Twalk;
	message->header.token =
			create_token(kcall->thread_get_id(), root->session);
	message->Twalk.tag = create_tag(req);
	message->Twalk.fid = root->node.key;
	message->Twalk.newfid = f->node.key;
	message->Twalk.nwname = len;
	message->Twalk.wname = &(req->walkpath[offset]);
//	log_info("proxy: walk %x %d [%s] %d\n", root->session->node.key,
//			message->Twalk.fid, message->Twalk.wname,
//			message->Twalk.newfid);

	result = call_device(root, req);
	if (result)
		session_destroy_file(f);
	else {
		*file = f;
		f->f_offset = 0;
		f->f_flag = O_ACCMODE;
		f->f_count = 1;
	}

	return result;
}

int _walk_child(mm_file_t *root, mm_file_t *file, const char *name,
		mm_request_t *req)
{
	size_t len = strlen(name);
	if (len >= PATH_MAX)
		//TODO test
		return ENAMETOOLONG;

	fsmsg_t *message = &(req->message);
	message->header.type = Twalk;
	message->header.token =
			create_token(kcall->thread_get_id(), root->session);
	message->Twalk.tag = create_tag(req);
	message->Twalk.fid = root->node.key;
	message->Twalk.newfid = file->node.key;
	message->Twalk.nwname = len;
	message->Twalk.wname = (char *) name;
	return call_device(root, req);
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

int _clunk(mm_file_t *file, const int token, mm_request_t *req)
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

	if (session_destroy_file(file)) {
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
	ER_UINT size = kcall->ipc_send(file->session->server_id, message,
			def->tsize);
	if (size)
		return ECONNREFUSED;

	req->callee = file->session->server_id;
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

int calc_path(char *dest, const char *wd, const char *src, const long size)
{
	if (size < 2)
		return ERR_INSUFFICIENT_BUFFER;

	if (!*src)
		return ERR_BAD_PATH;

	long rest = (long) size - 1;
	char *w;
	char *r = (char *) src;
	if (*r == PATH_DELIMITER) {
		w = dest;
		*w++ = PATH_DELIMITER;
		r++;
		rest--;
	} else {
		//TODO check format of wd. (should start with '/')
		long len = copy_path(dest, wd, rest);
		if (!len)
			return ERR_BAD_PATH;
		else if (len < 0)
			return ERR_INSUFFICIENT_BUFFER;

		w = &dest[len];
		rest -= len;

		if (w[-1] != PATH_DELIMITER) {
			if (rest <= 0)
				return ERR_INSUFFICIENT_BUFFER;

			*w++ = PATH_DELIMITER;
			rest--;
		}
	}

	for (;;) {
		switch (r[0]) {
		case PATH_NUL:
			goto end_of_src;
		case PATH_DELIMITER:
			r++;
			continue;
		case PATH_DOT:
			switch (r[1]) {
			case PATH_NUL:
				goto end_of_src;
			case PATH_DELIMITER:
				r += 2;
				continue;
			case PATH_DOT:
				switch (r[2]) {
				case PATH_NUL:
					w = omit_previous_entry(&rest, dest, w);
					if (w)
						goto end_of_src;
					else
						return ERR_BAD_PATH;
				case PATH_DELIMITER:
					w = omit_previous_entry(&rest, dest, w);
					if (w) {
						r += 3;
						continue;
					} else
						return ERR_BAD_PATH;
				default:
					break;
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		for (;;) {
			char c = *r;
			if (!c)
				goto end_of_src;

			if (rest <= 0)
				return ERR_INSUFFICIENT_BUFFER;

			*w++ = c;
			r++;
			rest--;

			if (c == PATH_DELIMITER)
				break;
		}
	}
end_of_src:

	{
		long len = (long) ((uintptr_t) w - (uintptr_t) dest);
		if (w[-1] == PATH_DELIMITER)
			if (len > 1) {
				w--;
				len--;
			}

		*w = PATH_NUL;
		return len;
	}
}

static long copy_path(char *dest, const char *src, const long size)
{
	char *w = dest;
	char *r = (char *) src;
	for (long rest = size; *r; rest--) {
		if (rest <= 0)
			return ERR_INSUFFICIENT_BUFFER;

		*w++ = *r++;
	}

	return ((uintptr_t) w - (uintptr_t) dest);
}

static char *omit_previous_entry(long *rest, char *head, char *tail)
{
	long len = (long) ((uintptr_t) tail - (uintptr_t) head);
	if (len <= 1)
		return NULL;

	char *p = (char *) tail - 3;
	for (; *p != PATH_DELIMITER; p--);

	len = (long) (((uintptr_t) tail - (uintptr_t) p)) - 1;
	*rest += len;
	return (tail - len);
}
