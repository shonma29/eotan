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
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <services.h>
#include <string.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/unistd.h>
#include "../../lib/libserv/libserv.h"
#include "pm.h"
#include "process.h"

static int _seek(mm_process_t *, mm_file_t *, mm_args_t *);
static size_t calc_path(char *, char *, const size_t);


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
