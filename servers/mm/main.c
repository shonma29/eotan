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
#include <device.h>
#include <errno.h>
#include <mm.h>
#include <pm.h>
#include <services.h>
#include <string.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "../../kernel/mpu/interrupt.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "interface.h"
#include "process.h"

static int (*funcs[])(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args) = {
	mm_palloc,
	mm_pfree,
	mm_vmap,
	mm_vunmap,
	mm_clock_gettime,
	mm_process_create,
	mm_process_clean,
	mm_process_duplicate,
	mm_process_set_context,
	mm_sbrk,
	mm_thread_create,
	mm_thread_find,
	mm_dup,
	mm_wait
};

#define BUFSIZ (sizeof(mm_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))
#define MYNAME "mm"

static char pathbuf1[PATH_MAX];
static char pathbuf2[PATH_MAX];

static ER init(void);
static ER proxy_initialize(void);
static void proxy(void);
static int if_fork(mm_process_t *, pm_args_t *);
static int if_exec(mm_process_t *, pm_args_t *);
static int if_chdir(mm_process_t *, pm_args_t *);
static size_t calc_path(char *, char *, const size_t);
static int if_open(mm_process_t *, pm_args_t *);
static int if_read(mm_process_t *, pm_args_t *);
static int if_write(mm_process_t *, pm_args_t *);
static int if_close(mm_process_t *, pm_args_t *);
static int file_close(mm_file_t *, devmsg_t *);
static int if_stat(mm_process_t *, pm_args_t *);
static int if_ref_fid(mm_process_t *, pm_args_t *);
static int call_device(const int, devmsg_t *, const size_t, const int,
		const size_t);
static void doit(void);

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	process_initialize();

	ER result = proxy_initialize();
	if (result)
		return result;

	define_mpu_handlers((FP)default_handler, (FP)stack_fault_handler);

	return kcall->port_open(&pk_cpor);
}

static ER proxy_initialize(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT, 0, proxy, pri_server_middle,
		KTHREAD_STACK_SIZE, NULL, NULL, NULL
	};

	return kcall->thread_create(PORT_PM, &pk_ctsk);
}

static void proxy(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, sizeof(pm_args_t), sizeof(pm_reply_t) };
	ER error = kcall->port_open(&pk_cpor);
	if (error) {
		log_err("proxy: open failed %d\n", error);
		return;
	} else
		log_info("proxy: start\n");

	for (;;) {
		RDVNO rdvno;
		pm_args_t args;
		ER_UINT size = kcall->port_accept(PORT_PM, &rdvno, &args);
		if (size < 0) {
			log_err("proxy: receive failed %d\n", size);
			break;
		}

		pm_reply_t *reply = (pm_reply_t*)(&args);
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO return error
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO thread has pointer to process
		}

		args.process_id |= get_rdv_tid(rdvno) << 16;

		int result;
		int op = args.operation;
		switch (args.operation) {
		case pm_syscall_fork:
			result = if_fork(process, &args);
			break;
		case pm_syscall_exec:
			result = if_exec(process, &args);
			break;
		case pm_syscall_exit:
			result = if_exit(process, &args);
			break;
		case pm_syscall_chdir:
			result = if_chdir(process, &args);
			break;
		case pm_syscall_open:
//TODO walk and open / create
			result = if_open(process, &args);
			break;
		case pm_syscall_close:
			result = if_close(process, &args);
			break;
		case pm_syscall_fstat:
			result = if_stat(process, &args);
			break;
		case pm_syscall_read:
			result = if_read(process, &args);
			break;
		case pm_syscall_write:
			result = if_write(process, &args);
			break;
		case pm_syscall_lseek:
//TODO implement in here
			result = if_ref_fid(process, &args);
			break;
		default:
			result = (kcall->port_call(PORT_FS, &args, sizeof(args))
					== sizeof(pm_reply_t))? 0:ECONNREFUSED;
			break;
		}

		if (result) {
			log_err("proxy: call %d failed %d\n", op, result);
			reply->result1 = -1;
			reply->result2 = 0;
			reply->error_no = result;
		}

		result = kcall->port_reply(rdvno, reply, sizeof(*reply));
		if (result)
			log_err("proxy: reply failed %d\n", result);
	}
}

int mm_dup(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
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

		int fid = args->arg1;
		mm_descriptor_t *d1 = process_find_desc(process, fid);
		if (!d1) {
			reply->data[0] = EBADF;
			break;
		}

		if (fid != args->arg2) {
			if (args->arg2 < 0) {
				reply->data[0] = EINVAL;
				break;
			}

			mm_descriptor_t *d2 = process_find_desc(process,
					args->arg2);
			if (d2) {
				pm_args_t pargs;
				pargs.operation = pm_syscall_close;
				pargs.process_id = process->node.key
						| get_rdv_tid(rdvno) << 16;
				file_close(d2->file, (devmsg_t*)&pargs);
				d2->file = NULL;
			} else {
				int fid2 = args->arg2;
				//TODO lock fid2 in tree
				d2 = process_allocate_desc();
				if (!d2) {
					reply->result = ENOMEM;
					break;
				}

				if (process_set_desc(process, fid2, d2)) {
					process_deallocate_desc(d2);
					//TODO what to do?
				}
			}

			d2->file = d1->file;
			d2->file->f_count++;
		}

		reply->result = args->arg2;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_wait(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
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

		process->rdvno = rdvno;

		int result = process_release_body(process);
		if (result) {
			reply->data[0] = result;
			break;
		}

		return reply_wait;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

//TODO to mm call
int if_fork(mm_process_t *process, pm_args_t *args)
{
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (!(reply->result1)) {
	}

	log_notice("proxy: %d fork %d %d\n",
			process->node.key, reply->result1, reply->error_no);
	return 0;
}

int if_exec(mm_process_t *process, pm_args_t *args)
{
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (!(reply->result1)) {
	}

	log_notice("proxy: %d exec %d %d\n",
			process->node.key, reply->result1, reply->error_no);
	return 0;
}

int if_exit(mm_process_t *process, pm_args_t *args)
{
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (!(reply->result1)) {
		process_destroy(process, args->arg1);
	}

	log_notice("proxy: %d exit %d %d\n",
			process->node.key, reply->result1, reply->error_no);
	return 0;
}

static int if_chdir(mm_process_t *process, pm_args_t *args)
{
	ER_UINT len = kcall->region_copy((args->process_id >> 16) & 0xffff,
			(char*)(args->arg1), PATH_MAX, pathbuf2);
	if (len < 0)
		return EFAULT;

	if (len >= PATH_MAX)
		return ENAMETOOLONG;

	strcpy(pathbuf1, process->local->wd);
	len = calc_path(pathbuf1, pathbuf2, PATH_MAX);
	if (!len)
		return ENAMETOOLONG;

	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (reply->result1 == 0) {
		process->local->wd_len = len;
		strcpy(process->local->wd, pathbuf1);
	}

	log_notice("proxy: %d chdir %d %d %s\n",
			process->node.key, reply->result1, reply->error_no,
			process->local->wd);

	return 0;
}

static size_t calc_path(char *dest, char *src, const size_t size)
{
	char *w;
	char *r = src;
	size_t rest = size;

	if (*r == '/') {
		w = dest;
		r++;
	} else {
		size_t last = strlen(dest);
		if (last == 1)
			last = 0;

		w = &(dest[last]);
		rest -= last;
	}

	bool last = false;
	do {
		char *word = r;
		for (;; r++) {
			if (!*r) {
				last = true;
				break;
			}

			if (*r == '/') {
				*r = '\0';
				break;
			}
		}

		size_t len = (size_t)r - (size_t)word;
		r++;

		if (!len)
			continue;

		if (!strcmp(word, "."))
			continue;

		if (!strcmp(word, "..")) {
			for (;;) {
				if (w == dest)
					break;
				w--;
				if (*w == '/')
					break;
			}

			rest = size - ((size_t)w - (size_t)dest);
			continue;
		}

		if (rest < len + 2)
			return 0;

		w[0] = '/';
		len++;
		strcpy(&(w[1]), word);
		w = &(w[len]);
		rest -= len;
	} while (!last);

	size_t total = size - rest;
	if (total)
		return total;
	else {
		dest[0] = '/';
		dest[1] = '\0';
		return 1;
	}
}

static int if_open(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_create_file();
	if (!d)
		return ENOMEM;

	int oflag = args->arg2;

	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		process_deallocate_file(d->file);
		process_deallocate_desc(d);
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	log_notice("proxy: %d open %d %d\n",
			process->node.key, reply->result1, reply->error_no);

	if (reply->result1 >= 0) {
		mm_file_t *f = d->file;
		f->server_id = PORT_FS;
		f->fid = reply->result1;
		f->f_flag = oflag;
		f->f_count = 1;
		//TODO set at last if append mode
		f->f_offset = 0;

		//TODO find empty fid
		if (process_set_desc(process, reply->result1, d)) {
			process_deallocate_file(d->file);
			process_deallocate_desc(d);
			//TODO what to do?
		}
	} else {
		process_deallocate_file(d->file);
		process_deallocate_desc(d);
	}

	return 0;
}

static int if_read(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_find_desc(process, args->arg1);
	if (!d)
		return EBADF;

	mm_file_t *f = d->file;
	devmsg_t *message = (devmsg_t*)args;
	if (f->server_id != PORT_FS)
		message->type = Tread;
	message->Tread.fid = f->fid;
	message->Tread.count = args->arg3;
	message->Tread.data = (char*)(args->arg2);
	message->Tread.offset = 0;

	int result = call_device(f->server_id, message, MESSAGE_SIZE(Tread),
			Rread, MESSAGE_SIZE(Rread));
	if (result)
		return result;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = args->arg1;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_write(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_find_desc(process, args->arg1);
	if (!d)
		return EBADF;

	mm_file_t *f = d->file;
	devmsg_t *message = (devmsg_t*)args;
	if (f->server_id != PORT_FS)
		message->type = Twrite;
	message->Twrite.fid = f->fid;
	message->Twrite.count = args->arg3;
	message->Twrite.data = (char*)(args->arg2);
	message->Twrite.offset = 0;

	int result = call_device(f->server_id, message, MESSAGE_SIZE(Twrite),
			Rwrite, MESSAGE_SIZE(Rwrite));
	if (result)
		return result;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = args->arg1;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_close(mm_process_t *process, pm_args_t *args)
{
	int fid = args->arg1;
	mm_descriptor_t *d = process_find_desc(process, fid);
	if (!d)
		return EBADF;

	int result = file_close(d->file, (devmsg_t*)args);
	d->file = NULL;
	if (process_destroy_desc(process, fid)) {
		//TODO what to do?
	}

	if (result) {
		log_notice("proxy: %d close err(%d) %d\n",
				process->node.key, fid, result);
		return result;
	}

	log_notice("proxy: %d close ok(%d)\n",
			process->node.key, fid);

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int file_close(mm_file_t *file, devmsg_t *message)
{
	file->f_count--;
	if (file->f_count > 0) {
		log_notice("proxy: close skip(%d)\n", file->fid);
		return 0;
	}

	if (file->server_id != PORT_FS)
		message->type = Tclunk;
	message->Tclunk.fid = file->fid;

	int result = call_device(file->server_id, message,
			MESSAGE_SIZE(Tclunk), Rclunk, MESSAGE_SIZE(Rclunk));

	process_deallocate_file(file);
	return result;
}

static int if_stat(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_find_desc(process, args->arg1);
	if (!d)
		return EBADF;

	mm_file_t *f = d->file;
	devmsg_t *message = (devmsg_t*)args;
	message->Tstat.fid = f->fid;
	int result = call_device(f->server_id, message, MESSAGE_SIZE(Tstat),
			Rstat, MESSAGE_SIZE(Rstat));
	if (result)
		return result;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_ref_fid(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_find_desc(process, args->arg1);
	if (!d)
		return EBADF;

	args->arg1 = d->file->fid;
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t))
		return ECONNREFUSED;

	pm_reply_t *reply = (pm_reply_t*)args;
	log_info("proxy: ref_fid %d %d %d\n",
			reply->result1, reply->result2, reply->error_no);
	return 0;
}

static int call_device(const int server_id, devmsg_t *message,
	const size_t tsize, const int rtype, const size_t rsize)
{
	ER_UINT size = kcall->port_call(server_id, message, tsize);
	if (size >= MIN_MESSAGE_SIZE) {
		//TODO check tag

		if (message->type == rtype) {
			if (size == rsize)
				return 0;
		} else if (message->type == Rerror) {
			if (size == MESSAGE_SIZE(Rerror))
				return message->Rerror.ename;
		}
	}

	return ECONNREFUSED;
}

static void doit(void)
{
	for (;;) {
		mm_args_t args;
		mm_reply_t reply;
		RDVNO rdvno;
		int result;
		ER_UINT size = kcall->port_accept(PORT_MM, &rdvno, &args);

		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			break;
		}

		if (size != sizeof(mm_args_t)) {
			reply.result = -1;
			reply.data[0] = EINVAL;
			result = reply_failure;

		} else if (args.syscall_no > NUM_OF_FUNCS) {
			reply.result = -1;
			reply.data[0] = ENOTSUP;
			result = reply_failure;

		} else
			result = funcs[args.syscall_no](&reply, rdvno, &args);

		switch (result) {
		case reply_success:
		case reply_failure:
			result = kcall->port_reply(rdvno, &reply,
					sizeof(reply));
			if (result != E_OK)
				log_err(MYNAME ": reply failed %d\n",
						result);
			break;
		default:
			break;
		}
	}
}

void start(VP_INT exinf)
{
	ER error = init();
	if (error)
		log_err(MYNAME ": open failed %d\n", error);
	else {
		log_info(MYNAME ": start\n");
		doit();
		log_info(MYNAME ": end\n");

		error = kcall->port_close();
		if (error)
			log_err(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
