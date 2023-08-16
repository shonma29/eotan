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
#include <init.h>
#include <string.h>
#include <nerve/ipc_utils.h>
#include <nerve/kcall.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include "mm.h"
#include "proxy.h"

static int cleanup(mm_process_t *, mm_thread_t *, mm_request_t *);
static int _seek(mm_process_t *, mm_file_t *, sys_args_t *, mm_request_t *);


int mm_fork(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_process_t *child = process_duplicate(process,
				(void *) (req->args.arg2),
				(void *) (req->args.arg1));
		if (!child) {
			log_err("pm: duplicate err\n");
			reply->data[0] = ENOMEM;
			break;
		}

//		log_info("pm: %d fork %d\n",
//				process->node.key, child->node.key);

		reply->result = child->node.key;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_exec(mm_request_t *req)
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

		result = _open(file,
				create_token(th->node.key, process->session),
				O_EXEC, req);
		if (result) {
			reply->data[0] = result;
//TODO clunk
			break;
		}

		int token = create_token(kcall->thread_get_id(),
				process->session);
		Elf32_Ehdr ehdr;
		result = _read(file, token, 0, sizeof(ehdr), (char *) &ehdr,
				req);
		if (result) {
			log_err("pm: ehdr0\n");
			reply->data[0] = result;
//TODO clunk
			break;
		} else {
			if (message->Rread.count == sizeof(ehdr)) {
				if (isValidModule(&ehdr)) {
//					log_info("pm: ehdr ok\n");
				} else {
					log_err("pm: ehdr1\n");
					reply->data[0] = ENOEXEC;
//TODO clunk
					break;
				}
			} else {
				log_err("pm: ehdr2\n");
				reply->data[0] = ENOEXEC;
//TODO clunk
				break;
			}
		}

		int x = 0;
		Elf32_Phdr ro;
		Elf32_Phdr rw;
		memset(&rw, 0, sizeof(rw));
		unsigned int offset = ehdr.e_phoff;
		for (int i = 0; i < ehdr.e_phnum; i++) {
			Elf32_Phdr phdr;
			result = _read(file, token, offset, sizeof(phdr),
					(char *) &phdr, req);
			if (result) {
//TODO clunk
//				return result;
				log_err("pm: phdr0\n");
				break;
			} else {
				if (message->Rread.count == sizeof(phdr)) {
//					log_info("pm: phdr ok\n");
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
//						log_info("pm: phdr all ok\n");
						break;
					}
				} else {
					log_err("pm: phdr2\n");
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
//		log_info("pm: replace\n");
		result = process_replace(process,
				(void *) (ro.p_vaddr),
				rw.p_memsz ? (rw.p_vaddr + rw.p_memsz
								- ro.p_vaddr)
						: ro.p_memsz,
				(void *) ehdr.e_entry,
				(void *) (req->args.arg2),
				req->args.arg3,
				&new_thread_id);
		if (result) {
			log_err("pm: replace0 %d\n", result);
			//TODO check error
		}

		token = create_token(new_thread_id, process->session);
		result = _read(file, token, ro.p_offset, ro.p_filesz,
				(char *) (ro.p_vaddr), req);
		if (result) {
			log_err("pm: tread0 %d\n", result);
		} else {
			if (message->Rread.count == ro.p_filesz) {
//				log_info("pm: tread1\n");
			} else {
				log_err("pm: tread2\n");
			}
		}

		if (rw.p_filesz) {
			result = _read(file, token, rw.p_offset, rw.p_filesz,
					(char *) (rw.p_vaddr), req);
			if (result) {
				log_err("pm: dread0 %d\n", result);
			} else {
				if (message->Rread.count == rw.p_filesz) {
//					log_info("pm: dread1\n");
				} else {
					log_err("pm: dread2\n");
				}
			}
		}

		kcall->thread_start(new_thread_id);

		//TODO recreate token (not must)
		result = _clunk(process->session, file, token, req);
		if (result) {
			log_err("pm: exec close1 %d\n", result);
			//TODO what to do?
		}

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_wait(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		//TODO support process group
		if ((req->args.arg1 < 0)
				&& (req->args.arg1 != (-1))) {
			reply->data[0] = EINVAL;
			break;
		}

		if (req->args.arg2
				&& (req->args.arg2 != WNOHANG)) {
			reply->data[0] = EINVAL;
			break;
		}

		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		process->tag = req->node.key;
		process->wpid = req->args.arg1;

		int result = process_release_body(process, req->args.arg2);
		if (result) {
			reply->data[0] = result;
			break;
		}

		return reply_wait;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_exit(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[0] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		cleanup(process, th, req);
		process_destroy(process, req->args.arg1);
//		log_info("pm: %d exit\n", process->node.key);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

static int cleanup(mm_process_t *process, mm_thread_t *th, mm_request_t *req)
{
	int token = create_token(th->node.key, process->session);
	for (node_t *node; (node = tree_root(&(process->descriptors)));) {
		int d = node->key;
		//TODO use getParent macro
		mm_descriptor_t *desc = (mm_descriptor_t *) node;
		mm_file_t *file = desc->file;
		desc->file = NULL;

		if (process_destroy_desc(process, d)) {
			//TODO what to do?
		}

		int fid = file->node.key;
		int result = _clunk(process->session, file, token, req);
		if (result)
			log_err("pm: %d cleanup[%d:%d] error=%d\n",
					process->node.key, d, fid, result);
	}

	if (process->wd) {
		mm_file_t *file = process->wd;
		process->wd = NULL;

		int fid = file->node.key;
		int result = _clunk(process->session, file, token, req);
		if (result)
			log_err("pm: %d cleanup[wd:%d] error=%d\n",
					process->node.key, fid, result);
	}

	return 0;
}

int mm_kill(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = NULL;
		if (req->args.arg1 == CURRENT_PROCESS)
			for (mm_process_t *p = process_find(INIT_PID); p;) {
				if (p->wpid)
					p = process_find(p->wpid);
				else {
					list_t *head = list_head(&(p->threads));
					if (head) {
						th = getMyThread(head);
						break;
					}
				}
			}
		else
			th = thread_find(req->args.arg1);

		if (!th) {
			reply->data[1] = EPERM;
			break;
		}

		mm_process_t *process = get_process(th);
		mm_thread_t *caller = thread_find(port_of_ipc(req->node.key));
		if (caller) {
			//TODO check privilege of caller
//			mm_process_t *caller_process = get_process(caller);
			reply->data[1] = EACCES;
			break;
		}

		log_info("pm: %d kill %d\n", process->node.key, req->args.arg2);

		//TODO write message to stderr
		cleanup(process, th, req);
		process_destroy(process, req->args.arg1);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_chdir(mm_request_t *req)
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

		struct stat st;
		result = _fstat(&st, file,
				create_token(kcall->thread_get_id(),
						process->session),
				req);

		int fid = file->node.key;
		int clunk_result = _clunk(process->session, file,
				create_token(th->node.key,
						process->session),
				req);
		if (clunk_result)
			//TODO what to do?
			log_err("pm: chdir close[:%d] error=%d\n",
					fid, clunk_result);

		if (result) {
			reply->data[0] = result;
			break;
		}

		if (!(st.st_mode & S_IFDIR)) {
			reply->data[0] = ENOTDIR;
			break;
		}

		process->local->wd_len = strlen(req->walkpath);
		strcpy(process->local->wd, req->walkpath);

//		log_info("pm: %d chdir %s\n",
//				process->node.key, process->local->wd);

		reply->result = 0;
		reply->data[0] = 0;
		return reply_success;
	} while (false);

	reply->result = -1;
	return reply_failure;
}

int mm_dup(mm_request_t *req)
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
				int error_no = _clunk(process->session,
						d2->file,
						create_token(th->node.key,
								process->session),
						req);
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

int mm_lseek(mm_request_t *req)
{
	sys_reply_t *reply = (sys_reply_t *) &(req->args);
	do {
		mm_thread_t *th = thread_find(port_of_ipc(req->node.key));
		if (!th) {
			reply->data[1] = EPERM;
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
		int result = _seek(process, desc->file, &(req->args), req);
		if (result) {
			reply->data[1] = result;
			break;
		}

//		log_info("pm: %d seek\n", process->node.key);

		off_t *offset = (off_t *) reply;
		*offset = desc->file->f_offset;

		reply->data[1] = 0;
		return reply_success;
	} while (false);

	off_t *offset = (off_t *) reply;
	*offset = (off_t) (-1);
	return reply_failure;
}

static int _seek(mm_process_t *process, mm_file_t *file, sys_args_t *args,
		mm_request_t *req)
{
	off_t *offset = (off_t *) &(args->arg2);
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
		int result = _fstat(&st, file,
				create_token(kcall->thread_get_id(),
						process->session),
				req);
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
