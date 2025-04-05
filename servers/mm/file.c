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
#include <set/sequence.h>
#include "process.h"

#define MIN_FID (1)
#define MIN_AUTO_FD (0)

enum {
	FILE_SLAB = 0x01,
	DESCRIPTOR_SLAB = 0x02,
	SESSION_SLAB = 0x04,
	SEQUENCE_SLAB = 0x08,
	SESSION_SEQUENCE = 0x10
};

static slab_t file_slab;
static slab_t descriptor_slab;
static slab_t session_slab;
static slab_t sequence_slab;
static tree_t session_tree;
static sequence_t session_sequence;
static int initialized_resources = 0;

static int process_find_new_fd(const mm_process_t *);
static int session_find_new_fid(mm_session_t *);
static mm_file_t *session_allocate_file(void);
static void session_deallocate_file(mm_file_t *);


int file_initialize(void)
{
	// initialize file table
	file_slab.unit_size = sizeof(mm_file_t);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_file_t));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	if (slab_create(&file_slab))
		return FILE_SLAB;
	else
		initialized_resources |= FILE_SLAB;

	// initialize descriptor table
	descriptor_slab.unit_size = sizeof(mm_descriptor_t);
	descriptor_slab.block_size = PAGE_SIZE;
	descriptor_slab.min_block = 1;
	descriptor_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_descriptor_t));
	descriptor_slab.palloc = kcall->palloc;
	descriptor_slab.pfree = kcall->pfree;
	if (slab_create(&descriptor_slab))
		return DESCRIPTOR_SLAB;
	else
		initialized_resources |= DESCRIPTOR_SLAB;

	// initialize session table
	session_slab.unit_size = sizeof(mm_session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = slab_max_block(SESSION_MAX, PAGE_SIZE,
			sizeof(mm_session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;
	if (slab_create(&session_slab))
		return SESSION_SLAB;
	else
		initialized_resources |= SESSION_SLAB;

	tree_create(&session_tree, NULL, NULL);

	// sequence slab is shared by many object types
	sequence_slab.unit_size = SEQUENCE_MAP_SIZE(PROCESS_MAX);
	sequence_slab.block_size = PAGE_SIZE;
	sequence_slab.min_block = 1;
	sequence_slab.max_block = slab_max_block(PROCESS_MAX * 2, PAGE_SIZE,
			SEQUENCE_MAP_SIZE(PROCESS_MAX));
	sequence_slab.palloc = kcall->palloc;
	sequence_slab.pfree = kcall->pfree;
	if (slab_create(&sequence_slab))
		return SEQUENCE_SLAB;
	else
		initialized_resources |= SEQUENCE_SLAB;

	void *buf = slab_alloc(&sequence_slab);
	if (!buf)
		return SESSION_SEQUENCE;

	if (sequence_initialize(&session_sequence, SESSION_MAX, buf)) {
		slab_free(&sequence_slab, buf);
		return SESSION_SEQUENCE;
	} else
		initialized_resources |= SESSION_SEQUENCE;

	// skip 0 as session id
	if (sequence_get(&session_sequence) < 0)
		return SESSION_SEQUENCE;

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
	int fid = session_find_new_fid(session);
	if (fid == -1)
		return NULL;

	mm_file_t *file = session_allocate_file();
	if (!file)
		return NULL;

	if (!tree_put(&(session->files), fid, &(file->node))) {
		session_deallocate_file(file);
		return NULL;
	}

	file->session = session;
	return file;
}

static int session_find_new_fid(mm_session_t *session)
{
	for (int fid = MIN_FID; fid < FILES_PER_SESSION; fid++)
		if (!tree_get(&(session->files), fid))
			return fid;

	return -1;
}

int session_destroy_file(mm_file_t *file)
{
	//TODO use other errno
	int result = tree_remove(&(file->session->files), file->node.key) ?
			0 : EBADF;

	if (!tree_size(&(file->session->files)))
		session_destroy(file->session);

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
