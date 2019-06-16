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
#include <fs/config.h>
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

static bool check_flags(const int flags);
static char *split_path(const char *path, char **parent_path);

//TODO split this file per function
int vfs_mount(const int device, vfs_t *fs, vnode_t *root)
{
	list_initialize(&(fs->vnodes));

	int error_no = fs->operations.mount(device, fs, root);
	if (error_no)
		return error_no;

	fs->root = root;
	root->fs = fs;
	vnodes_append(root);

	return 0;
}

int vfs_walk(vnode_t *parent, char *path, const int flags,
		const struct permission *perm, vnode_t **node)
{
	if (!parent)
		return ENODEV;

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

		int error_no = vfs_permit(parent, perm, R_OK | X_OK);
		if (!error_no)
			error_no = parent->fs->operations.walk(parent, head,
					node);

		vnodes_remove(parent);

		if (error_no)
			return error_no;

		parent = *node;

		if (last)
			break;
	}

	*node = parent;

	int error_no = vfs_permit(*node, perm, modes[flags & O_ACCMODE]);
	if (error_no)
		vnodes_remove(*node);

	return error_no;
}

int vfs_open(vnode_t *cwd, char *path, const int flags, const mode_t mode,
		struct permission *permission, vnode_t **node)
{
	if (!check_flags(flags)) {
		log_debug("vfs_open: bad flags %x\n", flags);
		return EINVAL;
	}

	if (flags & O_CREAT) {
		int error_no = vfs_create(cwd, path, flags, mode, permission,
				node);
		if (error_no != EEXIST)
			return error_no;
	}

	int error_no = vfs_walk(cwd, path, flags, permission, node);
	if (error_no)
		return error_no;

	if ((mode & S_IFMT) == S_IFDIR)
		if ((flags & O_ACCMODE) != O_RDONLY) {
			log_debug("vfs_open: %s is directory\n", path);
			vnodes_remove(*node);
			return EISDIR;
		}

	if (flags & O_TRUNC)
		(*node)->size = 0;

	return 0;
}

static bool check_flags(const int flags)
{
	switch (flags & O_ACCMODE) {
	case O_RDONLY:
	case O_WRONLY:
	case O_RDWR:
		break;
	default:
		return false;
	}

	//TODO unknown bits check is needed?
	if (flags & ~(O_ACCMODE | O_APPEND | O_CREAT | O_TRUNC))
		return false;

	return true;
}

int vfs_create(vnode_t *cwd, char *path, const int flags, const mode_t mode,
		const struct permission *permission, vnode_t **node)
{
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

	char *parent_path = "";
	char *head = split_path(path, &parent_path);
	if (!(*head)) {
		log_debug("vfs_create: bad path %s\n", path);
		return EINVAL;
	}

	vnode_t *parent;
	int result = vfs_walk(cwd, parent_path, O_WRONLY, permission,
			&parent);
	if (result) {
		log_debug("vfs_create: vfs_walk(%s) failed %d\n",
				parent_path, result);
		return result;
	}

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		log_debug("vfs_create: %s is not directory\n", parent_path);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	if (vfs_permit(parent, permission, W_OK | X_OK)) {
		log_debug("vfs_create: %s is not writable\n", parent_path);
		vnodes_remove(parent);
		return EACCES;
	}

	result = vfs_walk(parent, head, O_ACCMODE, permission, node);
	if (!result) {
		log_debug("vfs_create: %s already exists\n", head);
		vnodes_remove(*node);
		vnodes_remove(parent);
		return EEXIST;
	}

	if (mode & DMDIR)
		result = parent->fs->operations.mkdir(parent, head,
				//TODO really?
				mode & parent->mode
						& (S_IRWXU | S_IRWXG | S_IRWXO),
				permission, node);
	else
		result = parent->fs->operations.create(parent, head,
				//TODO really?
				mode & parent->mode
						& (S_IRUSR | S_IWUSR | S_IRGRP
						| S_IWGRP | S_IROTH | S_IWOTH),
				permission, node);

	vnodes_remove(parent);

	if (result) {
		log_debug("vfs_create: create(%s) failed %d\n", head, result);
		return result;
	}

	return 0;
}

int vfs_remove(vnode_t *cwd, char *path, const struct permission *permission)
{
	char *parent_path = "";
	char *head = split_path(path, &parent_path);
	if (!(*head)) {
		log_debug("vfs_remove: bad path %s\n", path);
		return EINVAL;
	}

	vnode_t *parent;
	int result = vfs_walk(cwd, parent_path, O_WRONLY, permission,
			&parent);
	if (result) {
		log_debug("vfs_remove: vfs_walk(%s) failed %d\n",
				parent_path, result);
		return result;
	}

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		log_debug("vfs_remove: %s is not directory\n", parent_path);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	if (vfs_permit(parent, permission, W_OK | X_OK)) {
		log_debug("vfs_remove: %s is not writable\n", parent_path);
		vnodes_remove(parent);
		return EACCES;
	}

	vnode_t *node;
	result = vfs_walk(parent, head, O_ACCMODE, permission, &node);
	if (result) {
		log_debug("vfs_remove: vfs_walk(%s) failed %d\n", head, result);
		vnodes_remove(parent);
		return result;
	}

	//TODO really?
	if (node->refer_count > 1) {
		log_debug("vfs_remove: %s is refered\n", head);
		vnodes_remove(node);
		vnodes_remove(parent);
		return EBUSY;
	}

	if ((node->mode & S_IFMT) == S_IFDIR)
		result = parent->fs->operations.rmdir(parent, head, node);
	else
		result = parent->fs->operations.remove(parent, head, node);

	vnodes_remove(node);
	vnodes_remove(parent);

	if (result) {
		log_debug("vfs_remove: mkdir(%s) failed %d\n", head, result);
		return result;
	}

	return 0;
}

static char *split_path(const char *path, char **parent_path)
{
	char *head = (char*)path;
	while (*head == '/')
		head++;

	char *last = strrchr(head, '/');
	if (last) {
		*last = '\0';
		*parent_path = head;
		head = last + 1;
	}

	return head;
}

int vfs_permit(const vnode_t *ip, const struct permission *permission,
		const unsigned int want)
{
	unsigned int mode = ip->mode;

	if (permission->uid == ROOT_UID) {
		mode |= S_IROTH | S_IWOTH | ((mode & S_IXUSR) >> 6)
				| ((mode & S_IXGRP) >> 3);
		if ((mode & S_IFMT) == S_IFDIR)
			mode |= S_IXOTH;
	} else if (permission->uid == ip->uid)
		mode >>= 6;
	else if (permission->gid == ip->gid)
		mode >>= 3;

	unsigned int need = want & (R_OK | W_OK | X_OK);
	return ((mode & need) == need)? 0:EACCES;
}

int vfs_read(vnode_t *ip, void *dest, const int offset, const size_t nbytes,
		size_t *read_len)
{
#ifdef UPDATE_ATIME
	SYSTIM clock;
	time_get(&clock);
#endif
	int result = ip->fs->operations.read(ip, dest, offset,
				(offset + nbytes > ip->size)?
						(ip->size - offset):nbytes,
				read_len);
#ifdef UPDATE_ATIME
	if (result >= 0) {
		ip->i_atime = clock;
		ip->i_dirty = true;
	}
#endif
	return result;
}
