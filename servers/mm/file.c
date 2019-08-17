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

	tree_create(&session_tree, NULL);
}

mm_session_t *session_create(void)
{
	int id;
	for (id = MIN_SID; id < SESSION_MAX; id++)
		if (!tree_get(&session_tree, id))
			break;

	if (id == SESSION_MAX)
		return NULL;

	mm_session_t *session = (mm_session_t*)slab_alloc(&session_slab);
	if (session) {
		if (!tree_put(&session_tree, id, &(session->node))) {
			//TODO what to do?
		}

		tree_create(&(session->files), NULL);
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

mm_descriptor_t *process_create_file(void)
{
	mm_descriptor_t *d = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (d) {
		mm_file_t *f = process_allocate_file();
		if (!f) {
			slab_free(&descriptor_slab, d);
			return NULL;
		}

		d->file = f;
	}

	return d;
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
		process_deallocate_file(desc->file);

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

int process_find_new_fd(const mm_process_t *process)
{
	for (int id = MIN_AUTO_FD; id < FILES_PER_PROCESS; id++)
		if (!process_find_desc(process, id))
			return id;

	return -1;
}

mm_descriptor_t *process_allocate_desc(void)
{
	mm_descriptor_t *d = (mm_descriptor_t*)slab_alloc(&descriptor_slab);
	if (d)
		//TODO is needed?
		d->file = NULL;

	return d;
}

void process_deallocate_desc(mm_descriptor_t *desc)
{
	slab_free(&descriptor_slab, desc);
}

int session_add_file(mm_session_t *session, const int fid, mm_file_t *file)
{
//log_info("mm: afile sid=%d fid=%d\n", session->node.key, fid);
	return (tree_put(&(session->files), fid, &(file->node)) ? 0 : 1);
}

int session_remove_file(mm_session_t *session, const int fid)
{
//log_info("mm: rfile sid=%d fid=%d\n", session->node.key, fid);
	mm_file_t *file = (mm_file_t*)tree_remove(&(session->files), fid);
	if (!file)
		return EBADF;

	return 0;
}

int session_find_new_fid(mm_session_t *session)
{
//log_info("mm: ffile %d\n", session->node.key);
	for (int fid = MIN_FID; fid < FILES_PER_SESSION; fid++)
		if (!tree_get(&(session->files), fid))
			return fid;

	return -1;
}

mm_file_t *process_allocate_file(void)
{
	return (mm_file_t*)slab_alloc(&file_slab);
}

void process_deallocate_file(mm_file_t *file)
{
	slab_free(&file_slab, file);
}

int create_tag(void)
{
	return 0;
}
