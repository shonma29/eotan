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
#include <fcntl.h>
#include <string.h>
#include <fs/vfs.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <sys/unistd.h>
#include "../../lib/libserv/libserv.h"

static int modes[] = {
	R_OK,
	W_OK,
	R_OK | W_OK,
	F_OK
};


//TODO split this file per function
int vfs_mount(const int device, vfs_t *fs, const size_t block_size)
{
	fs->device.channel = device;
	fs->device.block_size = block_size;
	if (block_initialize(&(fs->device)))
		return EIO;

	vnode_t *root = vnodes_create(NULL);
	if (!root)
		return E_NOMEM;

	list_initialize(&(fs->vnodes));
	fs->operations = vfs_fsops;

	int error_no = fs->operations.mount(fs, root);
	if (error_no) {
		vnodes_remove(root);
		return error_no;
	}

	fs->root = root;
	root->fs = fs;
	vnodes_append(root);
	return 0;

}

int vfs_unmount(vfs_t *fs)
{
	int error_no = vnodes_remove(fs->root);
	if (error_no)
		return error_no;

	error_no = fs->operations.unmount(fs);
	if (error_no)
		return error_no;

	return cache_flush(&(fs->device), true);
}

int vfs_walk(vnode_t *parent, char *path, const int flags,
		const struct permission *perm, vnode_t **node)
{
	if (!parent)
		return ENODEV;

	vnode_t *first = parent;
	char *p = (char*)path;
	for (parent->refer_count++;; p++) {
		bool last = false;
		char *head = p;
		int i;
		for (i = 0; i <= NAME_MAX; i++) {
			if (!(*p)) {
				last = true;
				break;
			} else if (*p == '/') {
				*p = '\0';
				break;
			} else
				p++;
		}
		if (!i) {
			if (last)
				break;
			else
				continue;
		} else if (i == (NAME_MAX + 1)) {
			vnodes_remove(parent);
			return ENAMETOOLONG;
		}

		if ((parent->mode & S_IFMT) != S_IFDIR) {
			vnodes_remove(parent);
			return ENOTDIR;
		}

		if (strcmp(head, "..")) {
			int error_no = vfs_permit(parent, perm, R_OK | X_OK);
			if (!error_no)
				error_no = parent->fs->operations.walk(parent,
						head, node);

			vnodes_remove(parent);

			if (error_no)
				return error_no;

			parent = *node;
		} else {
			vnode_t *pp = (parent->parent) ? parent->parent : first;
			pp->refer_count++;
			vnodes_remove(parent);
			parent = pp;
		}

		if (last)
			break;
	}

	*node = parent;

	int error_no = vfs_permit(*node, perm, modes[flags & O_ACCMODE]);
	if (error_no)
		vnodes_remove(*node);

	return error_no;
}

int vfs_open(vnode_t *vnode, const int flags, struct permission *permission)
{
	//TODO ad-hoc
	int f = (flags == O_EXEC) ? O_RDONLY : flags;
	if (!vfs_check_flags(f)) {
		log_debug("vfs_open: bad flags %x\n", flags);
		return EINVAL;
	}

	if ((vnode->mode & S_IFMT) == S_IFDIR)
		//TODO O_TRUNC and ORCLOSE are error
		if ((f & O_ACCMODE) != O_RDONLY) {
			log_debug("vfs_open: is directory\n");
			return EISDIR;
		}

	int error_no = vfs_permit(vnode, permission,
			(flags == O_EXEC) ? X_OK : modes[flags & O_ACCMODE]);
	if (error_no)
		return error_no;

	//TODO exclusive open

	//TODO need write permission
	if (f & O_TRUNC) {
		struct stat st;
		st.st_mode = (mode_t) (-1);
		st.st_gid = (gid_t) (-1);
		st.st_size = 0;
		st.st_mtime = -1;
		return vfs_wstat(vnode, &st);
	}

	return 0;
}

int vfs_create(vnode_t *parent, char *name, const int flags, const mode_t mode,
		const struct permission *permission, vnode_t **node)
{
	if ((parent->mode & S_IFMT) != S_IFDIR) {
		log_debug("vfs_create: [%d] is not directory\n", parent->index);
		return ENOTDIR;
	}

	if (vfs_permit(parent, permission, W_OK | X_OK)) {
		log_debug("vfs_create: [%d] is not writable\n", parent->index);
		return EACCES;
	}

	if (!vfs_is_valid_name(name)) {
		log_debug("vfs_create: bad name %s\n", name);
		return EINVAL;
	}

	//TODO really?
	if (mode & (UNMODIFIABLE_MODE_BITS & ~DMDIR)) {
		log_debug("vfs_create: bad mode %x\n", mode);
		return EINVAL;
	}

	//TODO really?
	if ((mode & DMDIR)
			&& ((flags & O_ACCMODE) != O_RDONLY)) {
		log_debug("vfs_create: bad mode %x\n", mode);
		return EINVAL;
	}

	int result = vfs_walk(parent, name, O_ACCMODE, permission, node);
	if (!result) {
		log_debug("vfs_create: %s already exists\n", name);
		vnodes_remove(*node);
		return EEXIST;
	}

//TODO extend permission from parent
	result = parent->fs->operations.create(parent, name, node);
	//TODO not close. open with flags
	if (result) {
		log_debug("vfs_create: create(%s) failed %d\n", name, result);
		return result;
	}

	if (mode & DMDIR) {
		(*node)->mode = (mode & parent->mode
				& (S_IRWXU | S_IRWXG | S_IRWXO))
				| S_IFDIR;
	} else {
		//TODO really?
		(*node)->mode = (mode & parent->mode
				& (S_IRUSR | S_IWUSR | S_IRGRP
						| S_IWGRP | S_IROTH | S_IWOTH))
				| S_IFREG;
	}

	(*node)->uid = permission->uid;
	(*node)->gid = permission->gid;
	(*node)->refer_count = 1;
	(*node)->lock_count = 0;
	return 0;
}

int vfs_remove(vnode_t *node, const struct permission *permission)
{
	vnode_t *parent = node->parent;
	if (!parent)
		//TODO really?
		return EINVAL;

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		log_debug("vfs_remove: [%d] is not directory\n", parent->index);
		return ENOTDIR;
	}

	if (vfs_permit(parent, permission, W_OK | X_OK)) {
		log_debug("vfs_remove: [%d] is not writable\n", parent->index);
		return EACCES;
	}

	//TODO really?
	if (node->refer_count > 1) {
		log_debug("vfs_remove: %d is referred\n", node->index);
		return EBUSY;
	}

	int result = parent->fs->operations.remove(parent, node);
	if (result) {
		log_debug("vfs_remove: remove(%d) failed %d\n", node->index,
				result);
		return result;
	}

	//TODO here?
	vnodes_remove(node);
	return 0;
}

int vfs_permit(const vnode_t *vnode, const struct permission *permission,
		const unsigned int want)
{
	unsigned int need = want & (R_OK | W_OK | X_OK);
	unsigned int bits = vnode->mode & need;

	if (permission->uid == vnode->uid)
		bits |= (vnode->mode >> 6) & need;

	if (permission->gid == vnode->gid)
		bits |= (vnode->mode >> 3) & need;

	return ((bits == need) ? 0 : EACCES);
}

int vfs_read(vnode_t *vnode, copier_t *dest, const unsigned int offset,
		const size_t nbytes, size_t *read_size)
{
	int result;
	do {
		if (offset > vnode->size) {
			result = EINVAL;
			break;
		}

		size_t rest = vnode->size - offset;
		size_t len = (nbytes > rest)? rest : nbytes;
		if (!len) {
			result = 0;
			break;
		}

		return vnode->fs->operations.read(vnode, dest, offset, len,
				read_size);
	} while (false);

	*read_size = 0;
	return result;
}
