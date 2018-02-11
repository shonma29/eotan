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
#include <string.h>
#include <fs/sfs.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include "func.h"
#include "../../lib/libserv/libserv.h"


int tfs_getdents(vnode_t *ip, struct dirent *entry, const int offset,
		const size_t max, size_t *length)
{
	int delta = 0;

	for (*length = 0; *length + sizeof(*entry) <= max;
			*length += sizeof(*entry)) {
		struct sfs_dir dir;
		size_t len;
		int error_no = sfs_i_read(ip, (B*)&dir, offset + delta,
				sizeof(dir), (W*)&len);
		if (error_no < 0)
			return error_no;

		if (!len)
			break;

		if (len != sizeof(dir))
			return EINVAL;

		entry->d_ino = dir.d_index;
		strncpy(entry->d_name, dir.d_name, SFS_MAXNAMELEN);
		entry->d_name[SFS_MAXNAMELEN] = '\0';

		delta += len;
		entry++;
	}

	return delta;
}

int tfs_walk(vnode_t *parent, const char *fname, vnode_t **retip)
{
	struct sfs_dir dir;
	size_t entries = parent->size / sizeof(dir);
	int i;
	for (i = 0; i < entries; i++) {
		size_t len;
		int error_no = sfs_i_read(parent, (B*)&dir, i * sizeof(dir),
				sizeof(dir), (W*)&len);
		if (error_no < 0)
			return error_no;

		if (!strncmp(fname, dir.d_name, SFS_MAXNAMELEN + 1))
			break;
	}
	if (i == entries)
		return ENOENT;

	*retip = vnodes_find(parent->fs, dir.d_index);
	if (*retip) {
		if ((*retip)->covered)
			*retip = (*retip)->covered;

		(*retip)->refer_count++;

		return 0;
	}

	*retip = vnodes_create();
	if (!(*retip))
		return ENOMEM;

	int error_no = sfs_read_inode(parent->fs, dir.d_index, *retip);
	if (error_no) {
		vnodes_remove(*retip);
		return error_no;
	}

	vnodes_append(*retip);

	return 0;
}

int tfs_mkdir(vnode_t *parent, const char *fname, const mode_t mode,
		struct permission *acc, vnode_t **retip)
{
	vnode_t *newip;
	int error_no = sfs_i_create(parent, (char*)fname, mode, acc, &newip);
	if (error_no)
		return error_no;

	struct sfs_dir dir[2] = {
		{ newip->index, "." },
		{ parent->index, ".." }
	};

	size_t len;
	error_no = sfs_i_write(newip, (B*)dir, 0, sizeof(dir), (W*)&len);
	if (error_no) {
		error_no = tfs_remove_entry(parent, fname, newip);
		if (error_no)
			dbg_printf("sfs_i_mkdir: %s is dead link\n", fname);

		sfs_free_inode(newip->fs, newip);
		vnodes_remove(newip);
		return (error_no);
	}

	newip->mode = mode | S_IFDIR;
	newip->nlink = 2;
	newip->dirty = true;

	parent->nlink += 1;
	parent->dirty = true;

	*retip = newip;

	return 0;
}

int tfs_append_entry(vnode_t *parent, const char *fname, vnode_t *ip)
{
	struct sfs_dir dir;

	dir.d_index = ip->index;
	strncpy(dir.d_name, fname, SFS_MAXNAMELEN);
	dir.d_name[SFS_MAXNAMELEN] = '\0';

	size_t len;
	return sfs_i_write(parent, (B*)&dir, parent->size, sizeof(dir),
			(W*)&len);
}

int tfs_remove_entry(vnode_t *parent, const char *fname, vnode_t *ip)
{
	struct sfs_dir dir;
	size_t entries = parent->size / sizeof(dir);
	int i;
	for (i = 0; i < entries; i++) {
		size_t len;
		int error_no = sfs_i_read(parent, (B*)&dir, i * sizeof(dir),
				sizeof(dir), (W*)&len);
		if (error_no < 0)
			return error_no;

		if (!strncmp(fname, dir.d_name, SFS_MAXNAMELEN + 1))
			break;
	}
	if (i == entries)
		return ENOENT;

	struct sfs_inode *sfs_inode = ip->private;
	ip->nlink--;
	time_get(&(sfs_inode->i_ctime));
	ip->dirty = true;

	for (i++; i < entries; i++) {
		size_t len;
		int error_no = sfs_i_read(parent, (B*)&dir, i * sizeof(dir),
				sizeof(dir), (W*)&len);
		if (error_no < 0)
			return error_no;

		error_no = sfs_i_write(parent, (B*)&dir, (i - 1) * sizeof(dir),
				sizeof(dir), (W*)&len);
		if (error_no)
			return error_no;
	}

	return sfs_i_truncate(parent, parent->size - sizeof(dir));
}
