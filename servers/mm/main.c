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
static int if_create(mm_process_t *, pm_args_t *);
static int if_read(mm_process_t *, pm_args_t *);
static int if_write(mm_process_t *, pm_args_t *);
static int if_close(mm_process_t *, pm_args_t *);
static int file_close(mm_session_t *, mm_file_t *, devmsg_t *);
static int if_remove(mm_process_t *, pm_args_t *);
static int if_stat(mm_process_t *, pm_args_t *);
static int if_wstat(mm_process_t *, pm_args_t *);
static int walk(mm_process_t *, const int, const char *, int *);
static void doit(void);

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	process_initialize();

	ER result = proxy_initialize();
	if (result)
		return result;

	define_mpu_handlers((FP)default_handler, (FP)stack_fault_handler);
//log_info("mm size=%d\n", BUFSIZ);
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

//	T_CPOR pk_cpor = { TA_TFIFO, sizeof(pm_args_t), sizeof(pm_reply_t) };
	T_CPOR pk_cpor = { TA_TFIFO, sizeof(pm_args_t), sizeof(devmsg_t) };
//log_info("proxy size=%d\n", sizeof(devmsg_t));
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

		args.process_id = process->session->node.key
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
			result = if_open(process, &args);
			break;
		case pm_syscall_create:
			result = if_create(process, &args);
			break;
		case pm_syscall_read:
			result = if_read(process, &args);
			break;
		case pm_syscall_write:
			result = if_write(process, &args);
			break;
		case pm_syscall_close:
			result = if_close(process, &args);
			break;
		case pm_syscall_remove:
			result = if_remove(process, &args);
			break;
		case pm_syscall_fstat:
			result = if_stat(process, &args);
			break;
		case pm_syscall_chmod:
			result = if_wstat(process, &args);
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
				file_close(process->session, d2->file,
						(devmsg_t*)&pargs);
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
log_info("exec0\n");
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th->process_id);
		if (!process) {
log_info("exec1\n");
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
		a.process_id = thread_id | process->session->node.key;
		a.arg1 = args->arg1;
		a.arg2 = O_EXEC;
		a.arg3 = 0;

		int fid = session_find_new_fid(process->session);
		if (fid == -1) {
			reply->data[0] = ENOMEM;
			break;
		}
		a.arg4 = fid;
//log_info("pexec0\n");
		if (kcall->port_call(PORT_FS, &a, sizeof(a))
				!= sizeof(pm_reply_t)) {
log_info("pexec1\n");
			reply->data[0] = ECONNREFUSED;
			break;
		}

		pm_reply_t *r = (pm_reply_t*)&a;
		if (r->result1 == -1) {
log_info("pexec 2\n");
			reply->data[0] = r->error_no;;
			break;
		}

		Elf32_Ehdr ehdr;
		a.operation = pm_syscall_read;
		devmsg_t *message = (devmsg_t*)&a;
		message->Tread.tag = (kcall->thread_get_id() << 16)
				| process->session->node.key;
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
//					log_info("ehdr ok\n");
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
					| process->session->node.key;
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
//					log_info("phdr ok\n");
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
//						log_info("phdr all ok\n");
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
//		log_info("replace\n");
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
				| process->session->node.key;
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
//				log_info("tread1\n");
			} else {
				log_err("tread2\n");
			}
		}

		if (rw.p_filesz) {
			a.operation = pm_syscall_read;
			message->Tread.tag = (new_thread_id << 16)
					| process->session->node.key;
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
//					log_info("dread1\n");
				} else {
					log_err("dread2\n");
				}
			}
		}

//		log_info("start\n");
		kcall->thread_start(new_thread_id);

		a.operation = pm_syscall_close;
		a.process_id = (new_thread_id << 16)
				| process->session->node.key;
		a.arg1 = fid;
		result = call_device(PORT_FS, (devmsg_t*)&a,
				MESSAGE_SIZE(Tclunk), Rclunk,
				MESSAGE_SIZE(Rclunk));
		if (result) {
			log_err("mm: exec close1 %d\n", result);
			//TODO what to do?
		}
//log_info("pexec s\n");
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

//		log_notice("mm: %d seek\n", process->node.key);

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
		int tag = (kcall->thread_get_id() << 16)
				| process->session->node.key;
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
	message.Tstat.fid = file->node.key;
	message.Tstat.stat = st;
	return call_device(file->server_id, &message, MESSAGE_SIZE(Tstat),
			Rstat, MESSAGE_SIZE(Rstat));
}

static int if_chdir(mm_process_t *process, pm_args_t *args)
{
	int tag = args->process_id;
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

	int fid = session_find_new_fid(process->session);
	if (fid == -1)
		return ENOMEM;

	mm_file_t *f = process_allocate_file();
	if (!f)
		return ENOMEM;

	args->arg2 = fid;
	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		process_deallocate_file(f);
		return ECONNREFUSED;
	}

	int old = (process->wd) ? process->wd->node.key : 0;
	pm_reply_t *reply = (pm_reply_t*)args;
	if (reply->result1 == 0) {
		process->local->wd_len = len;
		strcpy(process->local->wd, pathbuf1);

		f->server_id = PORT_FS;
		f->f_flag = O_ACCMODE;
		f->f_count = 1;
		f->f_offset = 0;

		if (process->wd) {
//log_info("mm: if_chdir0\n");
			pm_args_t a;
			a.operation = pm_syscall_close;
			a.process_id = (tag & 0xffff0000)
					| process->session->node.key;

			int result = file_close(process->session, process->wd,
					(devmsg_t*)&a);
log_info("if_chdir: close[%d] %d\n", old, result);
			if (result) {
				//TODO what to do?
			}

			process->wd = NULL;
		}

		if (session_add_file(process->session, fid, f)) {
			//TODO what to do?
		}

		process->wd = f;
	} else {
		process_deallocate_file(f);
	}

	log_notice("proxy: %d chdir %d %d %d->%d %s\n",
			process->node.key, reply->result1, reply->error_no,
			old, process->wd->node.key, process->local->wd);

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
	int fid = session_find_new_fid(process->session);
	if (fid == -1)
		return ENOMEM;

	int fd = process_find_new_fd(process);
	if (fd == -1)
		return ENOMEM;

	mm_descriptor_t *d = process_create_file();
	if (!d)
		return ENOMEM;

	int oflag = args->arg2;
	args->arg4 = fid;

	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		process_deallocate_file(d->file);
		process_deallocate_desc(d);
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	log_notice("proxy: %d open[%d:%d] %d %d\n",
			process->node.key, fd, fid, reply->result1,
			reply->error_no);

	if (reply->result1 >= 0) {
		mm_file_t *f = d->file;
		f->server_id = PORT_FS;
		f->f_flag = oflag;
		f->f_count = 1;
		//TODO set at last if append mode
		f->f_offset = 0;

		if (session_add_file(process->session, fid, f)) {
			//TODO what to do?
		}

		if (process_set_desc(process, fd, d)) {
			process_deallocate_file(d->file);
			process_deallocate_desc(d);
			//TODO what to do?
		}

		reply->result1 = fd;
	} else {
		process_deallocate_file(d->file);
		process_deallocate_desc(d);
	}

	return 0;
}

static int if_create(mm_process_t *process, pm_args_t *args)
{
	int fid = session_find_new_fid(process->session);
	if (fid == -1)
		return ENOMEM;

	int fd = process_find_new_fd(process);
	if (fd == -1)
		return ENOMEM;

	mm_descriptor_t *d = process_create_file();
	if (!d)
		return ENOMEM;

	int oflag = args->arg2;
	args->arg4 = fid;

	if (kcall->port_call(PORT_FS, args, sizeof(*args))
			!= sizeof(pm_reply_t)) {
		process_deallocate_file(d->file);
		process_deallocate_desc(d);
		return ECONNREFUSED;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	log_notice("proxy: %d create[%d:%d] %d %d\n",
			process->node.key, fd, fid, reply->result1,
			reply->error_no);

	if (reply->result1 >= 0) {
		mm_file_t *f = d->file;
		f->server_id = PORT_FS;
		//TODO really?
		f->f_flag = oflag & O_ACCMODE;
		f->f_count = 1;
		//TODO set at last if append mode
		f->f_offset = 0;

		if (session_add_file(process->session, fid, f)) {
			//TODO what to do?
		}

		if (process_set_desc(process, fd, d)) {
			process_deallocate_file(d->file);
			process_deallocate_desc(d);
			//TODO what to do?
		}

		reply->result1 = fd;
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
	message->Tread.fid = f->node.key;
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
	message->Twrite.fid = f->node.key;
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
	int fd = args->arg1;
	mm_descriptor_t *d = process_find_desc(process, fd);
	if (!d) {
		log_err("if_close: %d not found(%d)\n", process->node.key, fd);
		return EBADF;
	}

//log_info("mm: if_close0 fd=%d fid=%d\n", fd, d->file->node.key);
	int result = file_close(process->session, d->file, (devmsg_t*)args);
	d->file = NULL;
	if (process_destroy_desc(process, d->node.key)) {
		//TODO what to do?
	}

	if (result) {
		log_notice("proxy: %d close err(%d) %d\n",
				process->node.key, fd, result);
		return result;
	}

	log_notice("proxy: %d close ok(%d)\n",
			process->node.key, fd);

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int file_close(mm_session_t *session, mm_file_t *file, devmsg_t *message)
{
	file->f_count--;
	if (file->f_count > 0) {
		log_notice("proxy: close skip(%d)\n", file->node.key);
		return 0;
	}

	if (file->server_id != PORT_FS)
		message->type = Tclunk;
	message->Tclunk.fid = file->node.key;

	int result = call_device(file->server_id, message,
			MESSAGE_SIZE(Tclunk), Rclunk, MESSAGE_SIZE(Rclunk));

	if (session_remove_file(session, file->node.key)) {
		//TODO what to do?
	}

	process_deallocate_file(file);
	return result;
}

static int if_remove(mm_process_t *process, pm_args_t *args)
{
	int thread_id = (args->process_id >> 16) & 0xffff;
	int fid;
	int result = walk(process, thread_id, (char*)(args->arg1), &fid);
	if (result)
		return result;

	devmsg_t message;
	message.type = pm_syscall_remove;//Tremove;
	message.Tremove.tag = (thread_id << 16) | process->session->node.key;
	message.Tremove.fid = fid;

	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Tremove), Rremove, MESSAGE_SIZE(Rremove));
	//TODO unlock fid
	//TODO what todo if succeeded to walk and failed to remove
	if (result) {
		log_notice("proxy: %d remove err(%d) %d\n",
				process->node.key, fid, result);
		return result;
	}

	log_notice("proxy: %d remove ok(%d)\n",
			process->node.key, fid);

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_stat(mm_process_t *process, pm_args_t *args)
{
	mm_descriptor_t *d = process_find_desc(process, args->arg1);
	if (!d)
		return EBADF;

	mm_file_t *f = d->file;
	devmsg_t *message = (devmsg_t*)args;
	message->Tstat.fid = f->node.key;
	int result = call_device(f->server_id, message, MESSAGE_SIZE(Tstat),
			Rstat, MESSAGE_SIZE(Rstat));
	if (result) {
log_err("mm:if_stat result=%d\n", result);
		return result;
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_wstat(mm_process_t *process, pm_args_t *args)
{
	int thread_id = (args->process_id >> 16) & 0xffff;
	int fid;
	int result = walk(process, thread_id, (char*)(args->arg1), &fid);
	if (result) {
log_err("proxy: %d wstat0 err %d\n", process->node.key, result);
		return result;
	}

	struct stat st;
	st.st_mode = args->arg2;

	devmsg_t message;
	message.type = pm_syscall_chmod;//Twstat;
	message.Twstat.tag = (thread_id << 16) | process->session->node.key;
	message.Twstat.fid = fid;
	message.Twstat.stat = &st;

	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Twstat), Rwstat, MESSAGE_SIZE(Rwstat));
	//TODO unlock fid
//TODO clunk
	//TODO what todo if succeeded to walk and failed to remove
	if (result) {
		log_notice("proxy: %d wstat err(%d) %d\n",
				process->node.key, fid, result);
		return result;
	}

	log_notice("proxy: %d wstat ok(%d)\n",
			process->node.key, fid);

	message.type = pm_syscall_close;//Tclunk;
	message.Tclunk.tag = (thread_id << 16) | process->session->node.key;
	message.Tclunk.fid = fid;
	result = call_device(PORT_FS, &message,
			MESSAGE_SIZE(Tclunk), Rclunk, MESSAGE_SIZE(Rclunk));
	if (result) {
		log_err("proxy: %d fwstat close err(%d) %d\n",
				process->node.key, fid, result);
	}

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int walk(mm_process_t *process, const int thread_id, const char *path,
		int *fid)
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

	//TODO lock fid
	devmsg_t message;
	message.type = pm_syscall_walk;
	message.Twalk.tag = (thread_id << 16) | process->session->node.key;
	message.Twalk.fid = process->wd->node.key;
	message.Twalk.newfid = id;
	message.Twalk.nwname = len;
	message.Twalk.wname = (char*)path;
log_info(MYNAME ": walk %x %d %s %d\n", process->session,
		message.Twalk.fid, message.Twalk.wname,
		message.Twalk.newfid);

	int result;
	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Twalk), Rwalk, MESSAGE_SIZE(Rwalk));
log_info(MYNAME ": walk result=%d\n", result);

	if (!result)
		*fid = id;

	return result;
}

int call_device(const int server_id, devmsg_t *message,
	const size_t tsize, const int rtype, const size_t rsize)
{
	ER_UINT size = kcall->port_call(server_id, message, tsize);
//if (rtype == Rwalk)
//log_info(MYNAME ": call_device size=%d\n", size);
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
