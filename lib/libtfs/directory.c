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
#include <fs/tfs.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include "func.h"
#include "../../lib/libserv/libserv.h"

static int tfs_append_entry(vnode_t *, const char *, const ino_t);
static int tfs_remove_entry(vnode_t *, vnode_t *);


int tfs_getdents(vnode_t *parent, copier_t *dest, const unsigned int offset,
		const size_t max, size_t *length)
{
	unsigned int skip = offset / sizeof(struct dirent);
	if (offset % sizeof(struct dirent))
		return EINVAL;

	//TODO optimize
	unsigned int delta = 0;
	for (unsigned int i = 0; i < skip; i++) {
		char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN
				+ TFS_MAXNAMLEN];
		copier_t copier = {
			copy_to,
			buf
		};
		struct tfs_dir *dir = (struct tfs_dir *) buf;
		size_t len;
		int error_no = vfs_read(parent, &copier, delta, sizeof(buf),
				&len);
		if (error_no)
			return error_no;

		if (!len)
			break;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		delta += real_len;
	}

	for (*length = 0; *length + sizeof(struct dirent) <= max;
			*length += sizeof(struct dirent)) {
		char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN
				+ TFS_MAXNAMLEN];
		copier_t copier = {
			copy_to,
			buf
		};
		size_t len;
		int error_no = vfs_read(parent, &copier, delta, sizeof(buf),
				&len);
		if (error_no)
			return error_no;

		if (!len)
			break;

		struct tfs_dir *dir = (struct tfs_dir *) buf;
		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return EINVAL;

		size_t namlen = dir->d_namlen;
		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(namlen);
		if (len < real_len)
			return EINVAL;

		struct dirent *entry = (struct dirent *) buf;
		memmove(entry->d_name, dir->d_name, namlen);
		//TODO set 0 until end of buffer
		entry->d_name[namlen] = '\0';
		dest->copy(dest, entry, sizeof(*entry));

		delta += real_len;
	}

	return 0;
}

int tfs_walk(vnode_t *parent, const char *name, vnode_t **node)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	copier_t copier = {
		copy_to
	};
	struct tfs_dir *dir = (struct tfs_dir *) buf;
	size_t name_len = strlen(name);
	//TODO optimize
	for (unsigned int offset = 0;;) {
		copier.buf = buf;
		size_t len;
		int error_no = vfs_read(parent, &copier, offset, sizeof(buf),
				&len);
		if (error_no)
			return error_no;

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
			if (!memcmp(name, dir->d_name, name_len))
				break;

		offset += real_len;
	}

	*node = vnodes_find(parent->fs, dir->d_fileno);
	if (*node) {
		(*node)->refer_count++;

		return 0;
	}

	*node = vnodes_create(parent);
	if (!(*node))
		return ENOMEM;

	int error_no = tfs_open(parent->fs, dir->d_fileno, *node);
	if (error_no) {
		vnodes_remove(*node);
		return error_no;
	}

	vnodes_append(*node);

	return 0;
}

int tfs_create(vnode_t *parent, const char *name, vnode_t **node)
{
	vnode_t *vnode = vnodes_create(parent);
	if (!vnode)
		return ENOMEM;

	int error_no = tfs_allocate_inode(parent->fs, vnode);
	if (error_no) {
		vnodes_remove(vnode);
		return error_no;
	}

	vnodes_append(vnode);

	error_no = tfs_append_entry(parent, name, vnode->index);
	if (error_no) {
		tfs_deallocate_inode(parent->fs, vnode);
		vnodes_remove(vnode);
		return error_no;
	}

	SYSTIM clock;
	time_get(&clock);

	struct tfs_inode *inode = vnode->private;
	SYSTIM *p;
	p = (SYSTIM *) &(inode->i_atime);
	*p = clock;
	p = (SYSTIM *) &(inode->i_ctime);
	*p = clock;
	p = (SYSTIM *) &(inode->i_mtime);
	*p = clock;

	vnode->dirty = true;
	*node = vnode;
	return 0;
}

static int tfs_append_entry(vnode_t *parent, const char *name, const ino_t ino)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	struct tfs_dir *dir = (struct tfs_dir *) buf;
	size_t name_len = strlen(name);

	dir->d_fileno = ino;
	dir->d_namlen = name_len;
	memcpy(dir->d_name, name, name_len);

	copier_t copier = {
		copy_from,
		buf
	};
	size_t len;
	return tfs_write(parent, &copier, parent->size,
			sizeof(*dir) - TFS_MINNAMLEN + real_name_len(name_len),
			&len);
}

int tfs_remove(vnode_t *parent, vnode_t *vnode)
{
	if (vnode->refer_count > 1)
		return EBUSY;

	if (((vnode->mode & S_IFMT) == S_IFDIR)
			&& (vnode->size > 0))
		return ENOTEMPTY;

	int error_no = tfs_remove_entry(parent, vnode);
	if (error_no)
		return error_no;

	SYSTIM clock;
	time_get(&clock);

	struct tfs_inode *tfs_inode = parent->private;
	SYSTIM *p;
	p = (SYSTIM *) &(tfs_inode->i_mtime);
	*p = clock;
	p = (SYSTIM *) &(tfs_inode->i_ctime);
	*p = clock;
	parent->dirty = true;
	return tfs_deallocate_inode(vnode->fs, vnode);
}

static int tfs_remove_entry(vnode_t *parent, vnode_t *node)
{
	char buf[sizeof(struct tfs_dir) - TFS_MINNAMLEN + TFS_MAXNAMLEN];
	copier_t copier = {
		copy_to
	};
	struct tfs_dir *dir = (struct tfs_dir *) buf;
	unsigned int delta;
	unsigned int offset;
	//TODO optimize
	for (offset = 0;;) {
		copier.buf = buf;
		size_t len;
		int error_no = vfs_read(parent, &copier, offset, sizeof(buf),
				&len);
		if (error_no)
			return error_no;

		if (!len)
			return ENOENT;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			//TODO select adequate errno
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		if (dir->d_fileno == node->index) {
			delta = real_len;
			break;
		}

		offset += real_len;
	}

	//TODO optimize
	for (;;) {
		size_t len;
		int error_no = vfs_read(parent, &copier, offset + delta,
				sizeof(buf), &len);
		if (error_no)
			return error_no;

		if (!len)
			break;

		if (len < sizeof(*dir) - TFS_MINNAMLEN)
			return EINVAL;

		size_t real_len = sizeof(*dir) - TFS_MINNAMLEN
				+ real_name_len(dir->d_namlen);
		if (len < real_len)
			return EINVAL;

		copier_t writer = {
			copy_to,
			buf
		};
		error_no = tfs_write(parent, &writer, offset, real_len, &len);
		if (error_no)
			return error_no;

		offset += real_len;
	}

	return tfs_shorten(parent, parent->size - delta);
}
