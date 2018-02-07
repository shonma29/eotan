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
#include <fs/nconfig.h>
#include <fs/vfs.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <sys/unistd.h>
#include "../../lib/libserv/libserv.h"

static int modes[] = {
	R_OK,
	W_OK,
	R_OK | W_OK
};


int vfs_walk(vnode_t *parent, const char *path, const int flags,
		const struct permission *perm, vnode_t **ip)
{
	if (!parent)
		return ENODEV;

	for (parent->refer_count++; *path; path++) {
		char entry[NAME_MAX + 1];

		int i;
		for (i = 0; i < sizeof(entry); i++) {
			if ((*path == '/') || !(*path)) {
				entry[i] = '\0';
				break;
			} else {
				entry[i] = *path;
				path++;
			}
		}
		if (!i) {
			if (*path)
				continue;
			else
				break;
		} else if (i == sizeof(entry)) {
			vnodes_remove(parent);
			return ENAMETOOLONG;
		}

		if (!strcmp(entry, "..")) {
			vfs_t *fsp = parent->fs;

			if ((fsp->root == parent)
					&& fsp->origin) {
				vnodes_remove(parent);
				parent = fsp->origin;
				parent->refer_count++;
			}
		}

		if ((parent->mode & S_IFMT) != S_IFDIR) {
			vnodes_remove(parent);
			return ENOTDIR;
		}

		int error_no = vfs_permit(parent, perm, R_OK | X_OK);
		if (!error_no)
			error_no = parent->fs->operations.walk(parent, entry,
					ip);

		vnodes_remove(parent);

		if (error_no)
			return error_no;

		parent = *ip;

		if (!(*path))
			break;
	}

	*ip = parent;

	int error_no = vfs_permit(*ip, perm, modes[flags & O_ACCMODE]);
	if (error_no)
		vnodes_remove(*ip);

	return error_no;
}

int vfs_create(vnode_t *cwd, char *path, const mode_t mode,
		const struct permission *permission, vnode_t **ip)
{
	char *head = path;
	while (*head == '/')
		head++;

	char *last;
	for (;;) {
		last = strrchr(head, '/');
		if (!last)
			break;

		if (last[1] == '\0') {
			dbg_printf("vfs_create: bad path %s\n", path);
			return EINVAL;
		} else
			break;
	}

	char *parent_path;
	if (last) {
		*last = '\0';
		parent_path = head;
		head = last + 1;
	} else
		parent_path = "";

	if (!(*head)) {
		dbg_printf("vfs_create: bad path %s\n", path);
		return EINVAL;
	}

	vnode_t *parent;
	//TODO is O_WRONLY correct?
	int result = vfs_walk(cwd, parent_path, O_RDWR, permission,
			&parent);
	if (result) {
		dbg_printf("vfs_create: vfs_walk(%s) failed %d\n",
				parent_path, result);
		return result;
	}

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		dbg_printf("vfs_create: %s is not directory\n",
				parent_path);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	result = vfs_walk(parent, head, O_RDONLY, permission, ip);
	if (!result) {
		dbg_printf("vfs_create: %s already exists\n", head);
		vnodes_remove(*ip);
		vnodes_remove(parent);
		return EEXIST;
	}

	result = parent->fs->operations.create(parent, head,
			//TODO really?
			mode & parent->mode
					& (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
							| S_IROTH | S_IWOTH),
			permission, ip);

	vnodes_remove(parent);

	if (result) {
		dbg_printf("vfs_create: create(%s) failed %d\n", head, result);
		return result;
	}

	return 0;
}

int vfs_mkdir(vnode_t *cwd, char *path, const mode_t mode,
		const struct permission *permission, vnode_t **ip)
{
	char *head = path;
	while (*head == '/')
		head++;

	char *last;
	for (;;) {
		last = strrchr(head, '/');
		if (!last)
			break;

		//TODO is this error?
		if (last[1] == '\0')
			*last = '\0';
		else
			break;
	}

	char *parent_path;
	if (last) {
		*last = '\0';
		parent_path = head;
		head = last + 1;
	} else
		parent_path = "";

	if (!(*head)) {
		dbg_printf("vfs_mkdir: bad path %s\n", path);
		return EINVAL;
	}

	vnode_t *parent;
	//TODO is O_WRONLY correct?
	int result = vfs_walk(cwd, parent_path, O_RDWR, permission,
			&parent);
	if (result) {
		dbg_printf("vfs_mkdir: vfs_walk(%s) failed %d\n",
				parent_path, result);
		return result;
	}

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		dbg_printf("vfs_mkdir: %s is not directory\n", parent_path);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	result = vfs_walk(parent, head, O_RDONLY, permission, ip);
	if (!result) {
		dbg_printf("vfs_mkdir: %s already exists\n", head);
		vnodes_remove(*ip);
		vnodes_remove(parent);
		return EEXIST;
	}

	result = parent->fs->operations.mkdir(parent, head,
			//TODO really?
			mode & parent->mode & (S_IRWXU | S_IRWXG | S_IRWXO),
			permission, ip);

	vnodes_remove(parent);

	if (result) {
		dbg_printf("vfs_mkdir: mkdir(%s) failed %d\n", head, result);
		return result;
	}

	return 0;
}

int vfs_rmdir(vnode_t *cwd, char *path, const struct permission *permission)
{
	char *head = path;
	while (*head == '/')
		head++;

	char *last;
	for (;;) {
		last = strrchr(head, '/');
		if (!last)
			break;

		//TODO is this error?
		if (last[1] == '\0')
			*last = '\0';
		else
			break;
	}

	char *parent_path;
	if (last) {
		*last = '\0';
		parent_path = head;
		head = last + 1;
	} else
		parent_path = "";

	if (!(*head)) {
		dbg_printf("vfs_rmdir: bad path %s\n", path);
		return EINVAL;
	}

	vnode_t *parent;
	//TODO is O_WRONLY correct?
	int result = vfs_walk(cwd, parent_path, O_RDWR, permission,
			&parent);
	if (result) {
		dbg_printf("vfs_rmdir: vfs_walk(%s) failed %d\n",
				parent_path, result);
		return result;
	}

	if ((parent->mode & S_IFMT) != S_IFDIR) {
		dbg_printf("vfs_rmdir: %s is not directory\n", parent_path);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	vnode_t *ip;
	//TODO is O_RDONLY really?
	result = vfs_walk(parent, head, O_RDONLY, permission, &ip);
	if (result) {
		dbg_printf("vfs_rmdir: vfs_walk(%s) failed %d\n", head, result);
		vnodes_remove(parent);
		return result;
	}

	if ((ip->mode & S_IFMT) != S_IFDIR) {
		dbg_printf("vfs_rmdir: %s is not directory\n", head);
		vnodes_remove(ip);
		vnodes_remove(parent);
		return ENOTDIR;
	}

	if (ip->refer_count > 1) {
		dbg_printf("vfs_rmdir: %s is refered\n", head);
		vnodes_remove(ip);
		vnodes_remove(parent);
		return EBUSY;
	}

	result = parent->fs->operations.rmdir(parent, head, ip);
	vnodes_remove(ip);
	vnodes_remove(parent);

	if (result) {
		dbg_printf("vfs_rmdir: mkdir(%s) failed %d\n", head, result);
		return result;
	}

	return 0;
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
