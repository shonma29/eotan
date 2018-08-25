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


int tfs_getdents(vnode_t *parent, struct dirent *entry, const int offset,
		const size_t max, size_t *length)
{
	size_t delta = 0;

	//TODO optimize
	for (*length = 0; *length + sizeof(*entry) <= max;
			*length += sizeof(*entry)) {
		char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN
				+ TFS_MAXNAMLEN];
		struct tfs_dir *dir = (struct tfs_dir*)buf;
		size_t len;
		int error_no = sfs_i_read(parent, buf, offset + delta,
				sizeof(buf), (W*)&len);
		if (error_no < 0)
			return error_no;

		if (!len)
			break;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return (-EINVAL);

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return (-EINVAL);

		entry->d_ino = dir->d_fileno;
		memcpy(entry->d_name, dir->d_name, dir->d_namlen);
		//TODO set 0 until end of buffer
		entry->d_name[dir->d_namlen] = '\0';

		delta += real_len;
		entry++;
	}

	return delta;
}

int tfs_walk(vnode_t *parent, const char *name, vnode_t **node)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	struct tfs_dir *dir = (struct tfs_dir*)buf;
	size_t name_len = strlen(name);
	size_t delta;
	int offset;
	//TODO optimize
	for (offset = 0;;) {
		size_t len;
		int error_no = sfs_i_read(parent, buf, offset, sizeof(buf),
				(W*)&len);
		if (error_no < 0)
			return (-error_no);

		if (!len)
			return ENOENT;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		if (name_len == dir->d_namlen)
			if (!memcmp(name, dir->d_name, name_len)) {
				delta = real_len;
				break;
			}

		offset += real_len;
	}

	*node = vnodes_find(parent->fs, dir->d_fileno);
	if (*node) {
		if ((*node)->covered)
			*node = (*node)->covered;

		(*node)->refer_count++;

		return 0;
	}

	*node = vnodes_create();
	if (!(*node))
		return ENOMEM;

	int error_no = sfs_read_inode(parent->fs, dir->d_fileno, *node);
	if (error_no) {
		vnodes_remove(*node);
		return error_no;
	}

	vnodes_append(*node);

	return 0;
}

int tfs_mkdir(vnode_t *parent, const char *name, const mode_t mode,
		struct permission *permission, vnode_t **node)
{
	vnode_t *child;
	int error_no = sfs_i_create(parent, (char*)name, mode, permission,
			&child);
	if (error_no)
		return error_no;

	struct tfs_dir dir[] = {
		{ child->index, 1, ".\0" },
		{ parent->index, 2, ".." }
	};

	size_t len;
	error_no = sfs_i_write(child, (B*)dir, 0, sizeof(dir), (W*)&len);
	if (error_no) {
		error_no = tfs_remove_entry(parent, name, child);
		if (error_no)
			dbg_printf("sfs_i_mkdir: %s is dead link\n", name);

		sfs_free_inode(child->fs, child);
		vnodes_remove(child);
		return error_no;
	}

	child->mode = mode | S_IFDIR;
	child->nlink = 2;
	child->dirty = true;

	parent->nlink += 1;
	parent->dirty = true;

	*node = child;

	return 0;
}

int tfs_append_entry(vnode_t *parent, const char *name, vnode_t *node)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	struct tfs_dir *dir = (struct tfs_dir*)buf;
	size_t name_len = strlen(name);

	dir->d_fileno = node->index;
	dir->d_namlen = name_len;
	memcpy(dir->d_name, name, name_len);

	size_t len;
	return sfs_i_write(parent, buf, parent->size,
			sizeof(*dir) - TFS_MINNAMLEN + real_name_len(name_len),
			(W*)&len);
}

int tfs_remove_entry(vnode_t *parent, const char *name, vnode_t *node)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	struct tfs_dir *dir = (struct tfs_dir*)buf;
	size_t name_len = strlen(name);
	size_t delta;
	int offset;
	//TODO optimize
	for (offset = 0;;) {
		size_t len;
		int error_no = sfs_i_read(parent, buf, offset, sizeof(buf),
				(W*)&len);
		if (error_no < 0)
			return (-error_no);

		if (!len)
			return ENOENT;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		if (name_len == dir->d_namlen)
			if (!memcmp(name, dir->d_name, name_len)) {
				delta = real_len;
				break;
			}

		offset += real_len;
	}

	struct sfs_inode *sfs_inode = node->private;
	node->nlink--;
	time_get(&(sfs_inode->i_ctime));
	node->dirty = true;

	//TODO optimize
	for (;;) {
		size_t len;
		int error_no = sfs_i_read(parent, buf, offset + delta,
				sizeof(buf), (W*)&len);
		if (error_no < 0)
			return (-error_no);

		if (!len)
			break;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		error_no = sfs_i_write(parent, buf, offset, real_len,
				(W*)&len);
		if (error_no)
			return error_no;

		offset += real_len;
	}

	return sfs_i_truncate(parent, parent->size - delta);
}
