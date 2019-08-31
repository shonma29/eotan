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
#include "proxy.h"

static int _seek(mm_process_t *, mm_file_t *, sys_args_t *);
static size_t calc_path(char *, char *, const size_t);


int mm_fork(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_process_t *child = process_duplicate(process);
		if (!child) {
			log_err("mm: duplicate err\n");
			reply->data[0] = ENOMEM;
			break;
		}

		int thread_id;
		int error_no = thread_create(&thread_id, child,
				(FP)(req->args.arg2), (VP)(req->args.arg1));
		if (error_no) {
			log_err("mm: th create err\n");
			reply->data[0] = error_no;
			break;
		}

		if (kcall->thread_start(thread_id) < 0) {
			log_err("mm: th start err\n");
			//TODO use other errno
			reply->data[0] = ENOMEM;
			break;
		}

//		log_info("proxy: %d fork %d\n",
//				process->node.key, child->node.key);

		reply->result = child->node.key;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_exec(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		return process_exec(reply, process, th->node.key,
				&(req->args));
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int process_exec(sys_reply_t *reply, mm_process_t *process, const int thread_id,
		sys_args_t *args)
{
	do {
		mm_file_t *file;
		fsmsg_t message;
		int result = _walk(&file, process, thread_id,
				(char*)(args->arg1), &message);
		if (result) {
			reply->data[0] = result;
			break;
		}

		result = _open(file, create_token(thread_id, process->session),
				O_EXEC, &message);
		if (result) {
			reply->data[0] = result;
//TODO clunk
			break;
		}

		int token = create_token(kcall->thread_get_id(),
				process->session);
		Elf32_Ehdr ehdr;
		result = _read(file, token, 0, sizeof(ehdr), (char*)&ehdr,
				&message);
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
			result = _read(file, token, offset, sizeof(phdr),
					(char*)&phdr, &message);
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

		token = create_token(new_thread_id, process->session);
		result = _read(file, token, ro.p_offset, ro.p_filesz,
				(char*)(ro.p_vaddr), &message);
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
			result = _read(file, token, rw.p_offset, rw.p_filesz,
					(char*)(rw.p_vaddr), &message);
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

		//TODO recreate token (not must)
		result = _clunk(process->session, file, token, &message);
		if (result) {
			log_err("mm: exec close1 %d\n", result);
			//TODO what to do?
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	//TODO clunk
	reply->result = -1;
	return reply_failure;
}

int mm_wait(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		process->rdvno = req->rdvno;

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

int mm_exit(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		process_destroy(process, req->args.arg1);
//		log_info("mm: %d exit\n", process->node.key);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_chdir(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		ER_UINT len = kcall->region_copy(th->node.key,
				(char*)(req->args.arg1), PATH_MAX, pathbuf2);
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

//		int old = (process->wd) ? process->wd->node.key : 0;
		mm_file_t *file;
		fsmsg_t message;
		int result = _walk(&file, process, th->node.key,
				(char*)(req->args.arg1), &message);
		if (result) {
			reply->data[0] = result;
			break;
		}

		process->local->wd_len = len;
		strcpy(process->local->wd, pathbuf1);

		if (process->wd) {
			result = _clunk(process->session, process->wd,
					create_token(th->node.key,
							process->session),
					&message);
//			log_info("mm_chdir: close[:%d] %d\n", old, result);
			if (result) {
				//TODO what to do?
			}
		}

		process->wd = file;
//		log_info("proxy: %d chdir %d->%d %s\n",
//				process->node.key, old, process->wd->node.key,
//				process->local->wd);

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

int mm_dup(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[0] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		int fd = req->args.arg1;
		mm_descriptor_t *d1 = process_find_desc(process, fd);
		if (!d1) {
			reply->data[0] = EBADF;
			break;
		}

		if (fd != req->args.arg2) {
			if (req->args.arg2 < 0) {
				reply->data[0] = EINVAL;
				break;
			}

			mm_descriptor_t *d2 = process_find_desc(process,
					req->args.arg2);
			if (d2) {
				fsmsg_t message;
				int error_no = _clunk(process->session,
						d2->file,
						create_token(th->node.key,
								process->session),
						&message);
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

				if (process_set_desc(process, req->args.arg2,
						d2)) {
					process_deallocate_desc(d2);
					//TODO what to do?
				}
			}

			d2->file = d1->file;
			d2->file->f_count++;
		}

		reply->result = req->args.arg2;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_lseek(mm_request *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(get_rdv_tid(req->rdvno));
		if (!th) {
			//TODO use other errno
			reply->data[1] = ESRCH;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_descriptor_t *desc = process_find_desc(process,
				req->args.arg1);
		if (!desc) {
			reply->data[1] = EBADF;
			break;
		}

		//TODO return error if directory
		//TODO do nothing if pipe
		int result = _seek(process, desc->file, &(req->args));
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

static int _seek(mm_process_t *process, mm_file_t *file, sys_args_t *args)
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
		struct stat st;
		fsmsg_t message;
		int result = _fstat(&st, file,
				create_token(kcall->thread_get_id(),
						process->session),
				&message);
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
