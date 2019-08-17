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
#include "api.h"
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
	mm_open,
	mm_create,
	mm_read,
	mm_write,
	mm_close,
	mm_remove,
	mm_fstat,
	mm_chmod,
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
static int _clunk(mm_session_t *, mm_file_t *, devmsg_t *);
static int _walk(mm_process_t *, const int, const char *, mm_file_t **);
static void doit(void);

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	process_initialize();

	ER_ID result = proxy_initialize();
	if (result < 0)
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

	return kcall->thread_create_auto(&pk_ctsk);
}

static void proxy(void)
{
	sleep(1);
	exec_init(INIT_PID, INIT_PATH_NAME);
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
				message.header.token = (get_rdv_tid(rdvno) << 16)
						| process->session->node.key;
				message.Tclunk.tag = create_tag();
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
		message.header.type = Topen;
		message.header.token =
				(thread_id << 16) | process->session->node.key;
		message.Topen.tag = create_tag();
		message.Topen.fid = file->node.key;
		message.Topen.mode = O_EXEC;
		result = call_device(file->server_id, &message,
				MESSAGE_SIZE(Topen), Ropen,
				MESSAGE_SIZE(Ropen));
		if (result) {
			reply->data[0] = result;
//TODO clunk
			break;
		}

		Elf32_Ehdr ehdr;
		message.header.type = Tread;
		message.header.token = (kcall->thread_get_id() << 16)
				| process->session->node.key;
		message.Tread.tag = create_tag();
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
			message.header.type = Tread;
			message.header.token = (kcall->thread_get_id() << 16)
					| process->session->node.key;
			message.Tread.tag = create_tag();
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

		message.header.type = Tread;
		message.header.token = (new_thread_id << 16)
				| process->session->node.key;
		message.Tread.tag = create_tag();
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
			message.header.type =Tread;
			message.header.token = (new_thread_id << 16)
					| process->session->node.key;
			message.Tread.tag = create_tag();
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

		message.header.token = (new_thread_id << 16)
				| process->session->node.key;
		message.Tclunk.tag = create_tag();
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
		int token = (kcall->thread_get_id() << 16)
				| process->session->node.key;
		struct stat st;
		int result = _fstat(&st, file, token);
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

static int _fstat(struct stat *st, const mm_file_t *file, const int token)
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

		if (process->wd) {
			devmsg_t message;
			message.header.token = (th->node.key << 16)
					| process->session->node.key;
			message.Tclunk.tag = create_tag();
			result = _clunk(process->session, process->wd,
					&message);
			log_info("mm_chdir: close[:%d] %d\n", old, result);
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

static int _clunk(mm_session_t *session, mm_file_t *file, devmsg_t *message)
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
