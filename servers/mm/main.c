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
	mm_fork,
	mm_exec,
	mm_wait,
	mm_exit,
	mm_vmap,
	mm_vunmap,
	mm_sbrk,
	mm_chdir,
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
static int _seek(mm_process_t *, mm_file_t *, mm_args_t *);
static int _fstat(struct stat *, const mm_file_t *, const int);
static size_t calc_path(char *, char *, const size_t);
static char *split_path(const char *path, char **parent_path);
static int if_open(mm_process_t *, pm_args_t *);
static int if_create(mm_process_t *, pm_args_t *);
static int if_read(mm_process_t *, pm_args_t *);
static int if_write(mm_process_t *, pm_args_t *);
static int if_close(mm_process_t *, pm_args_t *);
static int _clunk(mm_session_t *, mm_file_t *, devmsg_t *);
static int if_remove(mm_process_t *, pm_args_t *);
static int if_stat(mm_process_t *, pm_args_t *);
static int if_wstat(mm_process_t *, pm_args_t *);
static int _walk(mm_process_t *, const int, const char *, mm_file_t **);
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

		int fd = args->arg1;
		mm_descriptor_t *d1 = process_find_desc(process, fd);
		if (!d1) {
			reply->data[0] = EBADF;
			break;
		}

		if (fd != args->arg2) {
			if (args->arg2 < 0) {
				reply->data[0] = EINVAL;
				break;
			}

			mm_descriptor_t *d2 = process_find_desc(process,
					args->arg2);
			if (d2) {
				devmsg_t message;
				message.Tclunk.tag = (get_rdv_tid(rdvno) << 16)
						| process->session->node.key;
				int error_no = _clunk(process->session,
						d2->file, &message);
				if (error_no) {
					//TODO what to do?
				}

				d2->file = NULL;
			} else {
				//TODO lock fd2 in tree
				d2 = process_allocate_desc();
				if (!d2) {
					reply->data[0] = ENOMEM;
					break;
				}

				if (process_set_desc(process, args->arg2, d2)) {
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

int mm_fork(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
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

		mm_process_t *child = process_duplicate(process);
		if (!child) {
			log_err("mm: duplicate err\n");
			reply->data[0] = ENOMEM;
			break;
		}

		int thread_id = thread_create(child, (FP)(args->arg2),
				(VP)(args->arg1));
		if (thread_id < 0) {
			log_err("mm: th create err\n");
			//TODO use other errno
			reply->data[0] = ENOMEM;
			break;
		}

		if (kcall->thread_start(thread_id) < 0) {
			log_err("mm: th start err\n");
			//TODO use other errno
			reply->data[0] = ENOMEM;
			break;
		}

		log_info("proxy: %d fork %d\n",
				process->node.key, child->node.key);

		reply->result = child->node.key;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
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
		return process_exec(reply, process, th->node.key, args);
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int process_exec(mm_reply_t *reply, mm_process_t *process, const int thread_id,
		mm_args_t *args)
{
	do {
		mm_file_t *file;
		int result = _walk(process, thread_id, (char*)(args->arg1),
				&file);
		if (result) {
			reply->data[0] = result;
			break;
		}

		devmsg_t message;
		message.type = pm_syscall_open;//Topen;
		message.Topen.tag =
				(thread_id << 16) | process->session->node.key;
		message.Topen.fid = file->node.key;
		message.Topen.mode = O_EXEC;
		result = call_device(process->wd->server_id, &message,
				MESSAGE_SIZE(Topen), Ropen,
				MESSAGE_SIZE(Ropen));
		if (result) {
			reply->data[0] = result;
//TODO clunk
			break;
		}

		Elf32_Ehdr ehdr;
		message.type = pm_syscall_read;//Tread;
		message.Tread.tag = (kcall->thread_get_id() << 16)
				| process->session->node.key;
		message.Tread.fid = file->node.key;
		message.Tread.count = sizeof(ehdr);
		message.Tread.data = (char*)&ehdr;
		message.Tread.offset = 0;

		result = call_device(PORT_FS, &message, MESSAGE_SIZE(Tread),
				Rread, MESSAGE_SIZE(Rread));
		if (result) {
			log_err("ehdr0\n");
			reply->data[0] = result;
//TODO clunk
			break;
		} else {
			if (message.Rread.count == sizeof(ehdr)) {
				if (isValidModule(&ehdr)) {
//					log_info("ehdr ok\n");
				} else {
					log_err("ehdr1\n");
					reply->data[0] = ENOEXEC;
//TODO clunk
					break;
				}
			} else {
				log_err("ehdr2\n");
				reply->data[0] = ENOEXEC;
//TODO clunk
				break;
			}
		}

		int x = 0;
		Elf32_Phdr ro;
		Elf32_Phdr rw;
		unsigned int offset = ehdr.e_phoff;
		for (int i = 0; i < ehdr.e_phnum; i++) {
			Elf32_Phdr phdr;
			message.type = pm_syscall_read;//Tread;
			message.Tread.tag = (kcall->thread_get_id() << 16)
					| process->session->node.key;
			message.Tread.fid = file->node.key;
			message.Tread.count = sizeof(phdr);
			message.Tread.data = (char*)&phdr;
			message.Tread.offset = offset;

			result = call_device(PORT_FS, &message,
					MESSAGE_SIZE(Tread),
					Rread, MESSAGE_SIZE(Rread));
			if (result) {
//TODO clunk
//				return result;
				log_err("phdr0\n");
				break;
			} else {
				if (message.Rread.count == sizeof(phdr)) {
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
//TODO clunk
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

		message.type = pm_syscall_read;//Tread;
		message.Tread.tag = (new_thread_id << 16)
				| process->session->node.key;
		message.Tread.fid = file->node.key;
		message.Tread.count = ro.p_filesz;
		message.Tread.data = (char*)(ro.p_vaddr);
		message.Tread.offset = ro.p_offset;

		result = call_device(PORT_FS, &message, MESSAGE_SIZE(Tread),
				Rread, MESSAGE_SIZE(Rread));
		if (result) {
			log_err("tread0 %d\n", result);
		} else {
			if (message.Rread.count == ro.p_filesz) {
//				log_info("tread1\n");
			} else {
				log_err("tread2\n");
			}
		}

		if (rw.p_filesz) {
			message.type = pm_syscall_read;//Tread;
			message.Tread.tag = (new_thread_id << 16)
					| process->session->node.key;
			message.Tread.fid = file->node.key;
			message.Tread.count = rw.p_filesz;
			message.Tread.data = (char*)(rw.p_vaddr);
			message.Tread.offset = rw.p_offset;

			result = call_device(PORT_FS, &message,
					MESSAGE_SIZE(Tread),
					Rread, MESSAGE_SIZE(Rread));
			if (result) {
				log_err("dread0 %d\n", result);
			} else {
				if (message.Rread.count == rw.p_filesz) {
//					log_info("dread1\n");
				} else {
					log_err("dread2\n");
				}
			}
		}

		kcall->thread_start(new_thread_id);

		message.Tclunk.tag = (new_thread_id << 16)
				| process->session->node.key;
		result = _clunk(process->session, file, &message);
		if (result) {
			log_err("mm: exec close1 %d\n", result);
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
		log_info("mm: %d exit\n", process->node.key);

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

//		log_info("mm: %d seek\n", process->node.key);

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

int mm_chdir(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
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
				(char*)(args->arg1), PATH_MAX, pathbuf2);
		if (len < 0) {
			reply->data[0] = EFAULT;
			break;
		}

	 	if (len >= PATH_MAX) {
			reply->data[0] = ENAMETOOLONG;
			break;
		}

		strcpy(pathbuf1, process->local->wd);
		len = calc_path(pathbuf1, pathbuf2, PATH_MAX);
		if (!len) {
			reply->data[0] = ENAMETOOLONG;
			break;
		}

		int old = (process->wd) ? process->wd->node.key : 0;
		mm_file_t *file;
		int result = _walk(process, th->node.key, (char*)(args->arg1),
				&file);
		if (result) {
			reply->data[0] = result;
			break;
		}

		process->local->wd_len = len;
		strcpy(process->local->wd, pathbuf1);

		file->server_id = PORT_FS;
		file->f_flag = O_ACCMODE;
		file->f_count = 1;
		file->f_offset = 0;

		if (process->wd) {
			devmsg_t message;
			message.Tclunk.tag = (th->node.key << 16)
					| process->session->node.key;
			result = _clunk(process->session, process->wd,
					&message);
			log_info("if_chdir: close[:%d] %d\n", old, result);
			if (result) {
				//TODO what to do?
			}
		}

		process->wd = file;
		log_info("proxy: %d chdir %d->%d %s\n",
				process->node.key, old, process->wd->node.key,
				process->local->wd);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
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

static int if_open(mm_process_t *process, pm_args_t *args)
{
	int fd = process_find_new_fd(process);
	if (fd == -1)
		return ENOMEM;

	mm_descriptor_t *d = process_allocate_desc();
	if (!d)
		return ENOMEM;

	if (process_set_desc(process, fd, d)) {
		process_deallocate_desc(d);
		//TODO use nother errno
		return ENOMEM;
	}

	int thread_id = (args->process_id >> 16) & 0xffff;
	mm_file_t *file;
	int result = _walk(process, thread_id, (char*)(args->arg1), &file);
	if (result) {
		process_destroy_desc(process, fd);
		return result;
	}

	d->file = file;
	int oflag = args->arg2;
	devmsg_t message;
	message.type = pm_syscall_open;//Topen;
	message.Topen.tag = (thread_id << 16) | process->session->node.key;
	message.Topen.fid = file->node.key;
	message.Topen.mode = oflag;

	//TODO find server_id
	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Topen), Ropen, MESSAGE_SIZE(Ropen));
	log_err("proxy: %d open[%d:%d] %d\n",
			process->node.key, fd, file->node.key, result);

	if (result) {
		d->file = NULL;
		process_destroy_desc(process, fd);

		message.Tclunk.tag = (thread_id << 16)
				| process->session->node.key;
		int error_no = _clunk(process->session, file, &message);
		if (error_no) {
			//TODO what to do?
		}

		return result;
	}

	file->server_id = process->wd->server_id;
	file->f_flag = oflag;
	file->f_count = 1;
	//TODO set at last if append mode
	file->f_offset = 0;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = fd;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int if_create(mm_process_t *process, pm_args_t *args)
{
	ER_UINT len = kcall->region_copy((args->process_id >> 16) & 0xffff,
			(char*)(args->arg1), PATH_MAX, pathbuf1);
	if (len < 0)
		return EFAULT;

	if (len >= PATH_MAX)
		return ENAMETOOLONG;

	int fd = process_find_new_fd(process);
	if (fd == -1)
		return ENOMEM;

	mm_descriptor_t *d = process_allocate_desc();
	if (!d)
		return ENOMEM;

	if (process_set_desc(process, fd, d)) {
		process_deallocate_desc(d);
		//TODO use nother errno
		return ENOMEM;
	}

	char *parent_path = "";
	char *head = split_path(pathbuf1, &parent_path);
	mm_file_t *file;
	int result = _walk(process, kcall->thread_get_id(), parent_path, &file);
	if (result) {
		process_destroy_desc(process, fd);
		return result;
	}

	int fid = file->node.key;
	int oflag = args->arg2;
	devmsg_t message;
	message.type = pm_syscall_create;//Tcreate;
	message.Tcreate.tag = (kcall->thread_get_id() << 16)
			| process->session->node.key;
	message.Tcreate.fid = fid;
	message.Tcreate.name = head;
	message.Tcreate.perm = args->arg3;
	message.Tcreate.mode = oflag;
	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Tcreate), Rcreate, MESSAGE_SIZE(Rcreate));
	log_info("proxy: %d create[%d:%d] %d\n",
			process->node.key, fd, fid, result);

	if (result) {
		process_destroy_desc(process, fd);

		message.Tclunk.tag = (kcall->thread_get_id() << 16)
				| process->session->node.key;
		int error_no = _clunk(process->session, file, &message);
		if (error_no) {
			//TODO what to do?
		}

		return result;
	}

	d->file = file;
	file->server_id = process->wd->server_id;
	//TODO really?
	file->f_flag = oflag & O_ACCMODE;
	file->f_count = 1;
	//TODO set at last if append mode
	file->f_offset = 0;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = fd;
	reply->result2 = 0;
	reply->error_no = 0;
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
	if (!d)
		return EBADF;

	mm_file_t *file = d->file;
	d->file = NULL;
	if (process_destroy_desc(process, fd)) {
		//TODO what to do?
	}

	devmsg_t message;
	message.Tclunk.tag = args->process_id;
	int result = _clunk(process->session, file, &message);

	if (result) {
		log_err("proxy: %d close[%d:] err %d\n",
				process->node.key, fd, result);
		return result;
	}

	log_info("proxy: %d close[%d:] ok\n",
			process->node.key, fd);

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int _clunk(mm_session_t *session, mm_file_t *file, devmsg_t *message)
{
	file->f_count--;
	if (file->f_count > 0) {
		log_info("proxy: close[:%d] skip\n", file->node.key);
		return 0;
	}

	message->type = (file->server_id == PORT_FS) ?
			pm_syscall_close : Tclunk;
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
	mm_file_t *file;
	int result = _walk(process, thread_id, (char*)(args->arg1), &file);
	if (result)
		return result;

	devmsg_t message;
	message.type = pm_syscall_remove;//Tremove;
	message.Tremove.tag = (thread_id << 16) | process->session->node.key;
	message.Tremove.fid = file->node.key;

	//TODO find server_id
	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Tremove), Rremove, MESSAGE_SIZE(Rremove));

	log_info("proxy: %d remove[:%d] %d\n",
			process->node.key, file->node.key, result);

	if (session_remove_file(process->session, file->node.key)) {
		//TODO what to do?
	}

	process_deallocate_file(file);

	if (result)
		return result;

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
	mm_file_t *file;
	int result = _walk(process, thread_id, (char*)(args->arg1), &file);
	if (result) {
		log_err("proxy: %d wstat0 err %d\n", process->node.key, result);
		return result;
	}

	struct stat st;
	st.st_mode = args->arg2;

	devmsg_t message;
	message.type = pm_syscall_chmod;//Twstat;
	message.Twstat.tag = (thread_id << 16) | process->session->node.key;
	message.Twstat.fid = file->node.key;
	message.Twstat.stat = &st;

	result = call_device(process->wd->server_id, &message,
			MESSAGE_SIZE(Twstat), Rwstat, MESSAGE_SIZE(Rwstat));

	int fid = file->node.key;
	log_info("proxy: %d wstat[:%d] %d\n",
			process->node.key, fid, result);
	message.Tclunk.tag = (thread_id << 16) | process->session->node.key;
	int error_no = _clunk(process->session, file, &message);
	if (error_no)
		log_err("proxy: %d fwstat close err[:%d] %d\n",
				process->node.key, fid, error_no);

	if (result)
		return result;

	pm_reply_t *reply = (pm_reply_t*)args;
	reply->result1 = 0;
	reply->result2 = 0;
	reply->error_no = 0;

	return 0;
}

static int _walk(mm_process_t *process, const int thread_id, const char *path,
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
	message.type = pm_syscall_walk;
	message.Twalk.tag = (thread_id << 16) | process->session->node.key;
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
