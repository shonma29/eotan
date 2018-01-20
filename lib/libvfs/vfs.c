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
#include <fs/vfs.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <sys/unistd.h>

static int modes[] = {
	R_OK,
	W_OK,
	R_OK | W_OK
};


int vfs_lookup(vnode_t *parent, const char *path, const int flags,
		const mode_t mode, const struct permission *perm, vnode_t **ip)
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

		if (!(parent->mode & S_IFDIR)) {
			vnodes_remove(parent);
			return ENOTDIR;
		}

		int error_no = parent->fs->operations.permit(parent, perm,
				R_OK | X_OK);
		if (!error_no)
			error_no = parent->fs->operations.lookup(parent, entry,
					ip);

		vnodes_remove(parent);

		if (error_no)
			return error_no;

		parent = *ip;

		if (!(*path))
			break;
	}

	*ip = parent;

	int error_no = (*ip)->fs->operations.permit(*ip, perm,
			modes[flags & 3]);
	if (error_no)
		vnodes_remove(*ip);

	return error_no;
}
