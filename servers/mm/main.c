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
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <mm.h>
#include <pm.h>
#include <services.h>
#include <string.h>
#include <boot/init.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "../../kernel/mpu/interrupt.h"
#include "../../kernel/mpu/mpufunc.h"
#include "../../lib/libserv/libserv.h"
#include "interface.h"
#include "process.h"

#define MIN_AUTO_FD (3)

static int (*funcs[])(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args) = {
	mm_exec,
	mm_wait,
	mm_exit,
	mm_vmap,
	mm_vunmap,
	mm_sbrk,
	mm_dup,
	mm_lseek,
	mm_clock_gettime,
	mm_thread_find
};

#define BUFSIZ (sizeof(mm_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))
#define MYNAME "mm"

static char pathbuf1[PATH_MAX];
static char pathbuf2[PATH_MAX];

static ER init(void);
static ER proxy_initialize(void);
static void proxy(void);
static unsigned int sleep(unsigned int);
static int if_fork(mm_process_t *, pm_args_t *);
static int _seek(mm_process_t *, mm_file_t *, mm_args_t *);
static int _fstat(struct stat *, const mm_file_t *, const int);
static int if_chdir(mm_process_t *, pm_args_t *);
static size_t calc_path(char *, char *, const size_t);
static int if_open(mm_process_t *, pm_args_t *);
static int if_read(mm_process_t *, pm_args_t *);
static int if_write(mm_process_t *, pm_args_t *);
static int if_close(mm_process_t *, pm_args_t *);
static int file_close(mm_file_t *, devmsg_t *);
static int if_stat(mm_process_t *, pm_args_t *);
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
	sleep(1);
	exec_init(INIT_PID, INIT_PATH_NAME);

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

		args.process_id = process->session_id
				| get_rdv_tid(rdvno) << 16;

		int result;
		int op = args.operation;
		switch (args.operation) {
		case pm_syscall_fork:
			result = if_fork(process, &args);
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
		default:
			result = (kcall->port_call(PORT_FS, &args, sizeof(args))
					== sizeof(pm_reply_t)) ?
							0 : ECONNREFUSED;
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

//TODO extract to libserv
static unsigned int sleep(unsigned int second)
{
	struct timespec t = { second, 0 };
	ER_UINT reply_size = kcall->port_call(PORT_TIMER, &t, sizeof(t));

	if (reply_size == sizeof(ER)) {
		ER *result = (ER*)&t;

		switch (*result) {
		case E_TMOUT:
			return 0;

		case E_PAR:
			return second;

		case E_NOMEM:
			return second;

		default:
			break;
		}
	}

	return second;
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
	mm_process_t *child = process_duplicate(process);
	if (!child) {
		log_err("mm: duplicate err\n");
		return ENOMEM;
	}

	int thread_id = thread_create(child, (FP)(args->arg2),
			(VP)(args->arg1));
	if (thread_id < 0) {
		log_err("mm: th create err\n");
		//TODO adequate errno
		return ENOMEM;
	}

	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	if (kcall->thread_start(thread_id) < 0) {
		log_err("mm: th start err\n");
		//TODO adequate errno
		return ENOMEM;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (!(reply->result1)) {
		reply->result1 = child->node.key;
	}

	log_notice("proxy: %d fork %d %d\n",
			process->node.key, reply->result1, reply->error_no);
	return 0;
}

int mm_exec(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
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
		return process_exec(reply, process, th->node.key << 16, args);
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int process_exec(mm_reply_t *reply, mm_process_t *process, const int thread_id,
		mm_args_t *args)
{
	do {
		pm_args_t a;
		a.operation = pm_syscall_open;
		a.process_id = thread_id | process->session_id;
		a.arg1 = args->arg1;
		a.arg2 = O_EXEC;
		a.arg3 = 0;

		if (kcall->port_call(PORT_FS, &a, sizeof(a))
				!= sizeof(pm_reply_t)) {
			reply->data[0] = ECONNREFUSED;
			break;
		}

		pm_reply_t *r = (pm_reply_t*)&a;
		if (r->result1 == -1) {
			reply->data[0] = r->error_no;;
			break;
		}

		int fid = r->result1;
		Elf32_Ehdr ehdr;
		a.operation = pm_syscall_read;
		devmsg_t *message = (devmsg_t*)&a;
		message->Tread.tag = (kcall->thread_get_id() << 16)
				| process->session_id;
		message->Tread.fid = fid;
		message->Tread.count = sizeof(ehdr);
		message->Tread.data = (char*)&ehdr;
		message->Tread.offset = 0;

		int result = call_device(PORT_FS, message, MESSAGE_SIZE(Tread),
				Rread, MESSAGE_SIZE(Rread));
		if (result) {
			log_err("ehdr0\n");
			reply->data[0] = result;
			break;
		} else {
			if (message->Rread.count == sizeof(ehdr)) {
				if (isValidModule(&ehdr)) {
					log_info("ehdr ok\n");
				} else {
					log_err("ehdr1\n");
					reply->data[0] = ENOEXEC;
					break;
				}
			} else {
				log_err("ehdr2\n");
				reply->data[0] = ENOEXEC;
				break;
			}
		}

		int x = 0;
		Elf32_Phdr ro;
		Elf32_Phdr rw;
		unsigned int offset = ehdr.e_phoff;
		for (int i = 0; i < ehdr.e_phnum; i++) {
			Elf32_Phdr phdr;
			a.operation = pm_syscall_read;
			message->Tread.tag = (kcall->thread_get_id() << 16)
					| process->session_id;
			message->Tread.fid = fid;
			message->Tread.count = sizeof(phdr);
			message->Tread.data = (char*)&phdr;
			message->Tread.offset = offset;

			result = call_device(PORT_FS, message, MESSAGE_SIZE(Tread),
					Rread, MESSAGE_SIZE(Rread));
			if (result) {
//				return result;
				log_err("phdr0\n");
				break;
			} else {
				if (message->Rread.count == sizeof(phdr)) {
					log_info("phdr ok\n");
					if (phdr.p_type != PT_LOAD)
						continue;
					switch (phdr.p_flags) {
					case (PF_X | PF_R):
						ro = phdr;
						x |= 1;
						break;
					case (PF_W | PF_R):
						rw = phdr;
						x |= 2;
						break;
					}
					if (x == 3) {
						log_info("phdr all ok\n");
						break;
					}
				} else {
					log_err("phdr2\n");
					//ENOEXEC
					break;
				}
			}
			offset += sizeof(phdr);
		}
//		if (x != 3)
//			return ENOEXEC;

		int new_thread_id;
		log_err("replace\n");
		result = process_replace(process,
				(void*)(ro.p_vaddr),
				rw.p_vaddr + rw.p_memsz - ro.p_vaddr,
				(void*)ehdr.e_entry, (void*)(args->arg2),
				args->arg3,
				&new_thread_id);
		if (result) {
			log_err("replace0 %d\n", result);
		}

		a.operation = pm_syscall_read;
		message->Tread.tag = (new_thread_id << 16)
				| process->session_id;
		message->Tread.fid = fid;
		message->Tread.count = ro.p_filesz;
		message->Tread.data = (char*)(ro.p_vaddr);
		message->Tread.offset = ro.p_offset;

		result = call_device(PORT_FS, message, MESSAGE_SIZE(Tread),
				Rread, MESSAGE_SIZE(Rread));
		if (result) {
			log_err("tread0 %d\n", result);
		} else {
			if (message->Rread.count == ro.p_filesz) {
				log_err("tread1\n");
			} else {
				log_err("tread2\n");
			}
		}

		if (rw.p_filesz) {
			a.operation = pm_syscall_read;
			message->Tread.tag = (new_thread_id << 16)
					| process->session_id;
			message->Tread.fid = fid;
			message->Tread.count = rw.p_filesz;
			message->Tread.data = (char*)(rw.p_vaddr);
			message->Tread.offset = rw.p_offset;

			result = call_device(PORT_FS, message, MESSAGE_SIZE(Tread),
					Rread, MESSAGE_SIZE(Rread));
			if (result) {
				log_err("dread0 %d\n", result);
			} else {
				if (message->Rread.count == rw.p_filesz) {
					log_err("dread1\n");
				} else {
					log_err("dread2\n");
				}
			}
		}

		log_err("start\n");
		kcall->thread_start(new_thread_id);

		a.operation = pm_syscall_close;
		a.process_id = (new_thread_id << 16) | process->session_id;
		a.arg1 = fid;
		if (kcall->port_call(PORT_FS, &a, sizeof(a))
				== sizeof(pm_reply_t)) {
			if (r->result1 == -1) {
				//TODO what to do?
				log_err("close0\n");
			}
		} else {
			log_err("close1\n");
			//TODO what to do?
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_exit(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
/*
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	if (!(reply->result1)) {
		process_destroy(process, args->arg1);
	}
*/
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

		process_destroy(process, args->arg1);
		log_notice("mm: %d exit\n", process->node.key);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_lseek(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		mm_thread_t *th = get_thread(get_rdv_tid(rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[1] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
			//TODO use other errno
			reply->data[1] = ESRCH;
			break;
		}

		mm_descriptor_t *desc = process_find_desc(process, args->arg1);
		if (!desc) {
			reply->data[1] = EBADF;
			break;
		}

		//TODO return error if directory
		//TODO do nothing if pipe
		int result = _seek(process, desc->file, args);
		if (result) {
			reply->data[1] = result;
			break;
		}

		log_notice("mm: %d seek\n", process->node.key);

		off_t *offset = (off_t*)reply;
		*offset = desc->file->f_offset;

		reply->data[1] = 0;
		return reply_success;
	} while (false);

	off_t *offset = (off_t*)reply;
	*offset = (off_t)(-1);
	return reply_failure;
}

static int _seek(mm_process_t *process, mm_file_t *file, mm_args_t *args)
{
	off_t *offset = (off_t*)&(args->arg2);
	off_t next = *offset;

	switch (args->arg4) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		if (next > 0) {
			off_t rest = LLONG_MAX - file->f_offset;
			if (next > rest)
				return EOVERFLOW;
		}

		next += file->f_offset;
		break;
	case SEEK_END:
	{
		int tag = (kcall->thread_get_id() << 16) | process->session_id;
		struct stat st;
		int result = _fstat(&st, file, tag);
		if (result)
			return result;

		if (next > 0) {
			off_t rest = LLONG_MAX - st.st_size;
			if (next > rest)
				return EOVERFLOW;
		}

		next += st.st_size;
	}
		break;
	default:
		return EINVAL;
	}

	if (next < 0)
		return EINVAL;

	file->f_offset = next;
	return 0;
}

static int _fstat(struct stat *st, const mm_file_t *file, const int tag)
{
	devmsg_t message;
	message.Tstat.tag = tag;
	message.Tstat.fid = file->fid;
	message.Tstat.stat = st;
	return call_device(file->server_id, &message, MESSAGE_SIZE(Tstat),
			Rstat, MESSAGE_SIZE(Rstat));
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

	int fid = process_find_new_fd(process);
	if (fid == -1)
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
		if (process_set_desc(process, fid, d)) {
			process_deallocate_file(d->file);
			process_deallocate_desc(d);
			//TODO what to do?
		}

		reply->result1 = fid;
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
	message->Tread.offset = f->f_offset;

	int result = call_device(f->server_id, message, MESSAGE_SIZE(Tread),
			Rread, MESSAGE_SIZE(Rread));
	if (result)
		return result;

	f->f_offset += message->Rread.count;

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
	message->Twrite.offset = f->f_offset;

	int result = call_device(f->server_id, message, MESSAGE_SIZE(Twrite),
			Rwrite, MESSAGE_SIZE(Rwrite));
	if (result)
		return result;

	f->f_offset += message->Rwrite.count;

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

int call_device(const int server_id, devmsg_t *message,
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
