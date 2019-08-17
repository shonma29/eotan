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
#include <mm.h>
#include <services.h>
#include <string.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"
#include "pm.h"
#include "process.h"

#define MYNAME "mm"

char pathbuf1[PATH_MAX];
char pathbuf2[PATH_MAX];

static char *split_path(const char *path, char **parent_path);


int attach(mm_process_t *process, const int thread_id)
{
	mm_session_t *session = session_create();
	if (!session)
		return ENOMEM;

	int fid = session_find_new_fid(session);
	if (fid == -1) {
		session_destroy(session);
		return ENOMEM;
	}

	mm_file_t *f = process_allocate_file();
	if (!f) {
		session_destroy(session);
		return ENOMEM;
	}

	devmsg_t message;
	message.header.type = Tattach;
	message.header.token = thread_id | session->node.key;
	message.Tattach.tag = create_tag();
	message.Tattach.fid = fid;
	message.Tattach.afid = NOFID;
	message.Tattach.uname = (char*)(process->uid);
	message.Tattach.aname = (char*)"/";

	int result = call_device(PORT_FS, &message, MESSAGE_SIZE(Tattach),
			Rattach, MESSAGE_SIZE(Rattach));
	if (result) {
		log_err("mm: attach(%x) %d\n", fid, result);
		process_deallocate_file(f);
		session_destroy(session);
		return result;
	}

	f->server_id = PORT_FS;
	f->f_flag = O_ACCMODE;
	f->f_count = 1;
	f->f_offset = 0;

	result = session_add_file(session, fid, f);
	if (result) {
		//TODO what to do?
	}
log_info("mm: attach(%p fid=%d) %d\n", session, fid, result);

	process->wd = f;
	process->session = session;

	return 0;
}

int mm_open(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		int fd = process_find_new_fd(process);
		if (fd == -1) {
			reply->data[0] = ENOMEM;
			break;
		}

		mm_descriptor_t *desc = process_allocate_desc();
		if (!desc) {
			reply->data[0] = ENOMEM;
			break;
		}

		if (process_set_desc(process, fd, desc)) {
			process_deallocate_desc(desc);
			//TODO use nother errno
			reply->data[0] = ENOMEM;
			break;
		}

		mm_file_t *file;
		int result = _walk(process, th->node.key, (char*)(args->arg1),
				&file);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		desc->file = file;
		int oflag = args->arg2;
		devmsg_t message;
		message.header.type = Topen;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Topen.tag = create_tag();
		message.Topen.fid = file->node.key;
		message.Topen.mode = oflag;
		result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Topen),
				Ropen, MESSAGE_SIZE(Ropen));
		log_err("proxy: %d open[%d:%d] %d\n",
				process->node.key, fd, file->node.key, result);

		if (result) {
			desc->file = NULL;
			process_destroy_desc(process, fd);

			message.header.token = (th->node.key << 16)
					| process->session->node.key;
			message.Tclunk.tag = create_tag();
			int error_no = _clunk(process->session, file, &message);
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

int mm_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		ER_UINT len = kcall->region_copy(th->node.key,
				(char*)(args->arg1), PATH_MAX, pathbuf1);
		if (len < 0) {
			reply->data[0] = EFAULT;
			break;
		}

		if (len >= PATH_MAX) {
			reply->data[0] = ENAMETOOLONG;
			break;
		}

		int fd = process_find_new_fd(process);
		if (fd == -1) {
			reply->data[0] = ENOMEM;
			break;
		}

		mm_descriptor_t *desc = process_allocate_desc();
		if (!desc) {
			reply->data[0] = ENOMEM;
			break;
		}

		if (process_set_desc(process, fd, desc)) {
			process_deallocate_desc(desc);
			//TODO use nother errno
			reply->data[0] = ENOMEM;
			break;
		}

		char *parent_path = "";
		char *head = split_path(pathbuf1, &parent_path);
		mm_file_t *file;
		int result = _walk(process, kcall->thread_get_id(),
				parent_path, &file);
		if (result) {
			process_destroy_desc(process, fd);
			reply->data[0] = result;
			break;
		}

		int fid = file->node.key;
		int oflag = args->arg2;
		devmsg_t message;
		message.header.type = Tcreate;
		message.header.token = (kcall->thread_get_id() << 16)
				| process->session->node.key;
		message.Tcreate.tag = create_tag();
		message.Tcreate.fid = fid;
		message.Tcreate.name = head;
		message.Tcreate.perm = args->arg3;
		message.Tcreate.mode = oflag;
		result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Tcreate),
				Rcreate, MESSAGE_SIZE(Rcreate));
		log_info("proxy: %d create[%d:%d] %d\n",
				process->node.key, fd, fid, result);

		if (result) {
			process_destroy_desc(process, fd);

			message.header.token = (kcall->thread_get_id() << 16)
					| process->session->node.key;
			message.Tclunk.tag = create_tag();
			int error_no = _clunk(process->session, file, &message);
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

static char *split_path(const char *path, char **parent_path)
{
	char *head = (char*)path;
	while (*head == '/')
		head++;

	char *last = strrchr(head, '/');
	if (last) {
		*last = '\0';
		*parent_path = head;
		head = last + 1;
	}

	return head;
}

int mm_read(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_descriptor_t *desc = process_find_desc(process, args->arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		mm_file_t *file = desc->file;
		devmsg_t message;
		message.header.type = Tread;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Tread.tag = create_tag();
		message.Tread.fid = file->node.key;
		message.Tread.offset = file->f_offset;
		message.Tread.count = args->arg3;
		message.Tread.data = (char*)(args->arg2);

		int result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Tread),
				Rread, MESSAGE_SIZE(Rread));
		if (result) {
			reply->data[0] = result;
			break;
		}

		file->f_offset += message.Rread.count;

		reply->result = message.Rread.count;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_write(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_descriptor_t *desc = process_find_desc(process, args->arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		mm_file_t *file = desc->file;
		devmsg_t message;
		message.header.type = Twrite;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Twrite.tag = create_tag();
		message.Twrite.fid = file->node.key;
		message.Twrite.offset = file->f_offset;
		message.Twrite.count = args->arg3;
		message.Twrite.data = (char*)(args->arg2);

		int result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Twrite),
				Rwrite, MESSAGE_SIZE(Rwrite));
		if (result) {
			reply->data[0] = result;
			break;
		}

		file->f_offset += message.Rwrite.count;

		reply->result = message.Rwrite.count;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_close(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		int fd = args->arg1;
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

		devmsg_t message;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Tclunk.tag = create_tag();
		int result = _clunk(process->session, file, &message);
		if (result) {
			log_err("proxy: %d close[%d:] err %d\n",
					process->node.key, fd, result);
			reply->data[0] = result;
			break;
		}

		log_info("proxy: %d close[%d:] ok\n",
				process->node.key, fd);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_remove(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_file_t *file;
		int result = _walk(process, th->node.key, (char*)(args->arg1),
				&file);
		if (result) {
			reply->data[0] = result;
			break;
		}

		devmsg_t message;
		message.header.type = Tremove;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Tremove.tag = create_tag();
		message.Tremove.fid = file->node.key;
		result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Tremove),
				Rremove, MESSAGE_SIZE(Rremove));

		log_info("proxy: %d remove[:%d] %d\n",
				process->node.key, file->node.key, result);

		if (session_remove_file(process->session, file->node.key)) {
			//TODO what to do?
		}

		process_deallocate_file(file);

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

int mm_fstat(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_descriptor_t *desc = process_find_desc(process, args->arg1);
		if (!desc) {
			reply->data[0] = EBADF;
			break;
		}

		int result = _fstat((struct stat *) args->arg2, desc->file,
				(th->node.key << 16)
						| process->session->node.key);
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

int mm_chmod(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_file_t *file;
		int result = _walk(process, th->node.key, (char*)(args->arg1),
				&file);
		if (result) {
			reply->data[0] = result;
			break;
		}

		struct stat st;
		st.st_mode = args->arg2;

		devmsg_t message;
		message.header.type = Twstat;
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Twstat.tag = create_tag();
		message.Twstat.fid = file->node.key;
		message.Twstat.stat = &st;
		result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Twstat),
				Rwstat, MESSAGE_SIZE(Rwstat));

		int fid = file->node.key;
		log_info("proxy: %d chmod[:%d] %d\n",
				process->node.key, fid, result);
		message.header.token = (th->node.key << 16)
				| process->session->node.key;
		message.Tclunk.tag = create_tag();
		int error_no = _clunk(process->session, file, &message);
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

int _walk(mm_process_t *process, const int thread_id, const char *path,
		mm_file_t **file)
{
	if (!(process->wd))
		//TODO what to do?
		return ECONNREFUSED;

	//TODO omit copy
	ER_UINT len = kcall->region_copy(thread_id, path, PATH_MAX, pathbuf2);
	if (len < 0)
		return EFAULT;

	if (len >= PATH_MAX)
		return ENAMETOOLONG;

	int id = session_find_new_fid(process->session);
	if (id == -1)
		return ENOMEM;

	mm_file_t *f = process_allocate_file();
	if (!f)
		return ENOMEM;

	if (session_add_file(process->session, id, f)) {
		process_deallocate_file(f);
		//TODO use other errno
		return ENOMEM;
	}

	devmsg_t message;
	message.header.type = Twalk;
	message.header.token = (thread_id << 16) | process->session->node.key;
	message.Twalk.tag = create_tag();
	message.Twalk.fid = process->wd->node.key;
	message.Twalk.newfid = id;
	message.Twalk.nwname = len;
	message.Twalk.wname = (char*)path;
	log_info(MYNAME ": walk %d %d [%s] %d\n", process->session->node.key,
			message.Twalk.fid, message.Twalk.wname,
			message.Twalk.newfid);

	int result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Twalk), Rwalk, MESSAGE_SIZE(Rwalk));
	if (result)
		process_deallocate_file(f);
	else {
		*file = f;
		f->server_id = process->wd->server_id;
		f->f_flag = O_ACCMODE;
		f->f_count = 1;
		f->f_offset = 0;
	}

	return result;
}

int _clunk(mm_session_t *session, mm_file_t *file, devmsg_t *message)
{
	file->f_count--;
	if (file->f_count > 0) {
		log_info("proxy: close[:%d] skip\n", file->node.key);
		return 0;
	}

	message->header.type = Tclunk;
	message->Tclunk.fid = file->node.key;

	int result = call_device(file->server_id, message,
			MESSAGE_SIZE(Tclunk), Rclunk, MESSAGE_SIZE(Rclunk));

	if (session_remove_file(session, file->node.key)) {
		//TODO what to do?
	}

	process_deallocate_file(file);
	return result;
}

int _fstat(struct stat *st, const mm_file_t *file, const int token)
{
	devmsg_t message;
	message.header.type = Tstat;
	message.header.token = token;
	message.Tstat.tag = create_tag();
	message.Tstat.fid = file->node.key;
	message.Tstat.stat = st;
	return call_device(file->server_id, &message, MESSAGE_SIZE(Tstat),
			Rstat, MESSAGE_SIZE(Rstat));
}

int call_device(const int server_id, devmsg_t *message,
	const size_t tsize, const int rtype, const size_t rsize)
{
	ER_UINT size = kcall->port_call(server_id, message, tsize);
//if (rtype == Rwalk)
//log_info(MYNAME ": call_device size=%d\n", size);
	if (size >= MIN_MESSAGE_SIZE) {
		//TODO check tag

		if (message->header.type == rtype) {
			if (size == rsize)
				return 0;
		} else if (message->header.type == Rerror) {
			if (size == MESSAGE_SIZE(Rerror))
				return message->Rerror.ename;
		}
	}

	return ECONNREFUSED;
}
