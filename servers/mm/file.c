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
#include "../../lib/libserv/libserv.h"

#define MIN_SID (1)
#define MIN_FID (1)
#define MIN_AUTO_FD (3)

static slab_t file_slab;
static slab_t descriptor_slab;
static slab_t session_slab;
static tree_t session_tree;

static int process_find_new_fd(const mm_process_t *);
static int session_find_new_fid(mm_session_t *);
static mm_file_t *session_allocate_file(void);


void file_initialize(void)
{
	// initialize file table
	file_slab.unit_size = sizeof(mm_file_t);
	file_slab.block_size = PAGE_SIZE;
	file_slab.min_block = 1;
	file_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_file_t));
	file_slab.palloc = kcall->palloc;
	file_slab.pfree = kcall->pfree;
	slab_create(&file_slab);

	// initialize descriptor table
	descriptor_slab.unit_size = sizeof(mm_descriptor_t);
	descriptor_slab.block_size = PAGE_SIZE;
	descriptor_slab.min_block = 1;
	descriptor_slab.max_block = slab_max_block(FILE_MAX, PAGE_SIZE,
			sizeof(mm_descriptor_t));
	descriptor_slab.palloc = kcall->palloc;
	descriptor_slab.pfree = kcall->pfree;
	slab_create(&descriptor_slab);

	// initialize session table
	session_slab.unit_size = sizeof(mm_session_t);
	session_slab.block_size = PAGE_SIZE;
	session_slab.min_block = 1;
	session_slab.max_block = slab_max_block(SESSION_MAX, PAGE_SIZE,
			sizeof(mm_session_t));
	session_slab.palloc = kcall->palloc;
	session_slab.pfree = kcall->pfree;
	slab_create(&session_slab);

	tree_create(&session_tree, NULL, NULL);
}

mm_session_t *session_create(void)
{
	int sid;
	for (sid = MIN_SID; sid < SESSION_MAX; sid++)
		if (!tree_get(&session_tree, sid))
			break;

	if (sid == SESSION_MAX)
		return NULL;

	mm_session_t *session = (mm_session_t*)slab_alloc(&session_slab);
	if (session) {
		if (!tree_put(&session_tree, sid, &(session->node))) {
			//TODO what to do?
		}

		tree_create(&(session->files), NULL, NULL);
		session->refer_count = 1;
	}

	return session;
}

int session_destroy(mm_session_t *session)
{
	session->refer_count--;
	if (!(session->refer_count)) {
		//TODO warn and release opened files
		//TODO clunk root fid if opened

		if (!tree_remove(&session_tree, session->node.key)) {
			//TODO what to do?
		}

		slab_free(&session_slab, session);
	}

	return 0;
}

mm_descriptor_t *process_create_dummy_file(void)
{
	mm_descriptor_t *desc = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (desc) {
		mm_file_t *file = session_allocate_file();
		if (!file) {
			slab_free(&descriptor_slab, desc);
			return NULL;
		}

		desc->file = file;
	}

	return desc;
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
			(mm_descriptor_t*)tree_remove(&(process->descriptors),
					fd);
	if (!desc)
		return EBADF;

	if (desc->file)
		//TODO close here?
		session_deallocate_file(desc->file);

	process_deallocate_desc(desc);
	return 0;
}

int process_set_desc(mm_process_t *process, const int fd, mm_descriptor_t *desc)
{
	return (tree_put(&(process->descriptors), fd, &(desc->node)) ? 0 : 1);
}

mm_descriptor_t *process_find_desc(const mm_process_t *process, const int fd)
{
	return (mm_descriptor_t*)tree_get(&(process->descriptors), fd);
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
	mm_descriptor_t *desc = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (desc)
		//TODO is needed?
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

	return file;
}

static int session_find_new_fid(mm_session_t *session)
{
	for (int fid = MIN_FID; fid < FILES_PER_SESSION; fid++)
		if (!tree_get(&(session->files), fid))
			return fid;

	return -1;
}

int session_destroy_file(mm_session_t *session, mm_file_t *file)
{
	//TODO use other errno
	int result = tree_remove(&(session->files), file->node.key) ? 0 : EBADF;

	session_deallocate_file(file);
	return result;
}

static mm_file_t *session_allocate_file(void)
{
	return (mm_file_t*)slab_alloc(&file_slab);
}

void session_deallocate_file(mm_file_t *file)
{
	slab_free(&file_slab, file);
}
