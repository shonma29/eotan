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
#include <mm/config.h>
#include <nerve/kcall.h>
#include "process.h"

#define MIN_AUTO_FD (0)

enum {
	FILE_SLAB = 0x01,
	FILE_SEQUENCE = 0x02,
	DESCRIPTOR_SLAB = 0x04,
	SESSION_SLAB = 0x08,
	SESSION_SEQUENCE = 0x10
};

static slab_t file_slab;
static sequence_t file_sequence;
static slab_t descriptor_slab;
static slab_t session_slab;
static tree_t session_tree;
static sequence_t session_sequence;
static int initialized_resources = 0;

static int process_find_new_fd(const mm_process_t *);
static mm_file_t *session_allocate_file(void);
static void session_deallocate_file(mm_file_t *);


int file_initialize(void)
{
	if (create_slab(&file_slab, sizeof(mm_file_t), FILE_MAX))
		return FILE_SLAB;
	else
		initialized_resources |= FILE_SLAB;

	if (create_id_store(&file_sequence, FILE_MAX))
		return FILE_SEQUENCE;
	else
		initialized_resources |= FILE_SEQUENCE;

	if (create_slab(&descriptor_slab, sizeof(mm_descriptor_t), FILE_MAX))
		return DESCRIPTOR_SLAB;
	else
		initialized_resources |= DESCRIPTOR_SLAB;

	if (create_slab(&session_slab, sizeof(mm_session_t), SESSION_MAX))
		return SESSION_SLAB;
	else
		initialized_resources |= SESSION_SLAB;

	tree_create(&session_tree, NULL, NULL);

	if (create_id_store(&session_sequence, SESSION_MAX))
		return SESSION_SEQUENCE;
	else
		initialized_resources |= SESSION_SEQUENCE;

	return 0;
}

mm_session_t *session_create(const int server_id)
{
	int sequence = sequence_get(&session_sequence);
	if (sequence < 0)
		return NULL;

	int sid = create_sid(server_id, sequence);
	mm_session_t *session = (mm_session_t *) slab_alloc(&session_slab);
	if (session) {
		if (!tree_put(&session_tree, sid, &(session->node))) {
			sequence_release(&session_sequence, sequence);
			slab_free(&session_slab, session);
			return NULL;
		}

		tree_create(&(session->files), NULL, NULL);
	}

	return session;
}

int session_destroy(mm_session_t *session)
{
	if (!tree_remove(&session_tree, session->node.key)) {
		//TODO what to do?
	}

	sequence_release(&session_sequence,
			sequence_from_sid(session->node.key));
	slab_free(&session_slab, session);
	return 0;
}

mm_descriptor_t *process_create_desc(mm_process_t *process)
{
	int fd = process_find_new_fd(process);
	if (fd == -1)
		return NULL;

	mm_descriptor_t *desc = process_allocate_desc();
	if (!desc)
		return NULL;

	if (process_set_desc(process, fd, desc)) {
		process_deallocate_desc(desc);
		return NULL;
	}

	return desc;
}

int process_destroy_desc(mm_process_t *process, const int fd)
{
	mm_descriptor_t *desc =
			(mm_descriptor_t *) tree_remove(&(process->descriptors),
					fd);
	if (!desc)
		return EBADF;

	if (desc->file)
		//TODO close here?
		session_destroy_file(desc->file);

	process_deallocate_desc(desc);
	return 0;
}

int process_set_desc(mm_process_t *process, const int fd, mm_descriptor_t *desc)
{
	return (tree_put(&(process->descriptors), fd, &(desc->node)) ? 0 : 1);
}

mm_descriptor_t *process_find_desc(const mm_process_t *process, const int fd)
{
	return (mm_descriptor_t *) tree_get(&(process->descriptors), fd);
}

static int process_find_new_fd(const mm_process_t *process)
{
	for (int fd = MIN_AUTO_FD; fd < FILES_PER_PROCESS; fd++)
		if (!process_find_desc(process, fd))
			return fd;

	return -1;
}

mm_descriptor_t *process_allocate_desc(void)
{
	mm_descriptor_t *desc = (mm_descriptor_t *) slab_alloc(&descriptor_slab);
	desc->file = NULL;
	return desc;
}

void process_deallocate_desc(mm_descriptor_t *desc)
{
	slab_free(&descriptor_slab, desc);
}

mm_file_t *session_create_file(mm_session_t *session)
{
	int fid = sequence_get(&file_sequence);
	if (fid == -1)
		return NULL;

	mm_file_t *file = session_allocate_file();
	if (!file) {
		sequence_release(&file_sequence, fid);
		return NULL;
	}

	if (!tree_put(&(session->files), fid, &(file->node))) {
		sequence_release(&file_sequence, fid);
		session_deallocate_file(file);
		return NULL;
	}

	file->session = session;
	return file;
}

int session_destroy_file(mm_file_t *file)
{
	//TODO use other errno
	int result = tree_remove(&(file->session->files), file->node.key) ?
			0 : EBADF;

	if (!tree_size(&(file->session->files)))
		session_destroy(file->session);

	sequence_release(&file_sequence, file->node.key);
	session_deallocate_file(file);
	return result;
}

static mm_file_t *session_allocate_file(void)
{
	return (mm_file_t *) slab_alloc(&file_slab);
}

static void session_deallocate_file(mm_file_t *file)
{
	slab_free(&file_slab, file);
}
