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
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../../include/sys/stat.h"
#include "../../include/fs/config.h"
#include "../../include/fs/tfs.h"
#include "../../include/fs/vfs.h"
#include "libserv.h"
#include "writer.h"

#define ERR_ARG (1)
#define ERR_MEMORY (2)
#define ERR_FILE (3)
#define ERR_UNKNOWN (4)

//TODO fix another way
#define DMDIR 0x80000000

typedef struct {
	uint32_t dev;
	uint32_t ino;
	uint32_t mode;
	uint32_t nlink;
	uint32_t uid;
	uint32_t gid;
	uint32_t rdev;
	int64_t size;
	int64_t atime;
	int64_t mtime;
	int64_t ctime;
	uint32_t blksize;
	int32_t blocks;
} vstat_t;

typedef struct {
	int32_t d_ino;
	char d_name[256];
} vdirent_t;

static vfs_t fs;
static struct permission permission = { ROOT_UID, ROOT_GID };


void *palloc(void)
{
	return malloc(4096);
}

void pfree(void *p)
{
	free(p);
}

void syslog(const int priority, const char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vprintf(message, ap);
}

int time_get(systime_t *p)
{
	p->sec = time(NULL);
	p->nsec = 0;

	return 0;
}

static int initialize(void)
{
	int result = cache_initialize();
	if (result) {
		printf("cache_initialize failed %d\n", result);
		return ERR_MEMORY;
	}

	result = vnodes_initialize(palloc, pfree, 65536);
	if (result) {
		printf("vnodes_initialize failed %d\n", result);
		return ERR_MEMORY;
	}

//	printf("initialize: done\n");

	return 0;
}

static int mount(vfs_t *fs, const char *filename, const size_t block_size)
{
	int fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("file open(%s) failed %d\n", filename, errno);
		return ERR_FILE;
	}

	int result = vfs_mount(fd, fs, block_size);
	if (result) {
		printf("mount(%d) failed %d\n", fd, result);
		close(fd);
		return ERR_UNKNOWN;
	}

//	printf("mount: fd = %d\n", fd);

	return 0;
}

static int unmount(vfs_t *fs)
{
	int result = vfs_unmount(fs);
	if (result) {
		printf("unmount failed %d\n", result);
		close(fs->device.channel);
		return ERR_UNKNOWN;
	}

	if (close(fs->device.channel)) {
		printf("file(%d) close failed %d\n", fs->device.channel, errno);
		return ERR_FILE;
	}

//	printf("unmount: done = %d\n", fs->device.channel);

	return 0;
}

static void format(const vstat_t *st, const char *path)
{
	char buf[13];
	strftime(buf, sizeof(buf), "%b %e %H:%M", localtime(&(st->mtime)));
	printf("%10o %3d %4d %4d %8llu %s %s\n",
			st->mode,
			st->nlink,
			st->uid,
			st->gid,
			st->size,
			buf,
			path);
}

static int do_stat(vnode_t *vnode, const char *path)
{
	//TODO use struct definition of guest
	vstat_t st;
	int result = vnode->fs->operations.stat(vnode, &st);
	if (result) {
		printf("stat(%d) failed %d\n", vnode->index, result);
		return ERR_UNKNOWN;
	}

	format(&st, path);

	return 0;
}

static int readdir(vfs_t *fs, vnode_t *vnode,
		const struct permission *permission)
{
	vdirent_t *buf = malloc(sizeof(vdirent_t));
	if (!buf) {
		printf("readdir: malloc failed\n");
		return ERR_MEMORY;
	}

	//TODO use off_t
//	off_t offset = 0;
	size_t offset = 0;
	copier_t copier = {
		copy_to,
	};
	for (;;) {
		copier.buf = (char *) buf;
		size_t read_size;
		int result = fs->operations.getdents(vnode, &copier, offset,
				sizeof(*buf), &read_size);
		if (result) {
			printf("readdir: getdents(%d, %d) failed %d\n",
					vnode->index, offset, result);
			free(buf);
			return ERR_FILE;
		}

		if (!read_size)
			break;

		for (int i = 0; i < read_size / sizeof(*buf); i++) {
			vnode_t *entry;
			//TODO use constant definition of guest
			result = vfs_walk(vnode, buf[i].d_name, O_RDONLY,
					permission, &entry);
			if (result) {
				printf("readdir: vfs_walk(%s) failed %d\n",
						buf[i].d_name, result);
				free(buf);
				return ERR_UNKNOWN;
			}

			result = do_stat(entry, buf[i].d_name);
			vnodes_remove(entry);

			if (result) {
				printf("readdir: stat(%s) failed %d\n",
						buf[i].d_name, result);
				free(buf);
				return ERR_UNKNOWN;
			}
		}

		offset += read_size;
	}

	free(buf);

	return 0;
}

static int do_ls(vfs_t *fs, char *path, const struct permission *permission)
{
	vnode_t *vnode;
	//TODO use constant definition of guest in vfs
	int result = vfs_walk(fs->root, path, O_RDONLY, permission, &vnode);
	if (result) {
		printf("ls: vfs_walk(%s) failed %d\n", path, result);
		return ERR_UNKNOWN;
	}

	//TODO use constant definition of guest, or define method in vnodes
	if ((vnode->mode & S_IFMT) == S_IFDIR) {
//		printf("%s is directory\n", path);
//		do_stat(vnode, path);
//		printf("---\n");
		result = readdir(fs, vnode, permission);
		vnodes_remove(vnode);

		if (result) {
			printf("ls: readdir(%d) failed %d\n",
					vnode->index, result);
			return ERR_UNKNOWN;
		}
	} else {
		result = do_stat(vnode, path);
		vnodes_remove(vnode);

		if (result) {
			printf("ls: stat(%s) failed %d\n", path, result);
			return ERR_UNKNOWN;
		}
	}

	return 0;
}

static int do_create(vfs_t *fs, char *path, const char *from,
		const struct permission *permission)
{
	char *parent_path = "";
	char *name = vfs_split_path(path, &parent_path);
	vnode_t *parent;
	int result = vfs_walk(fs->root, parent_path, O_ACCMODE, permission,
			&parent);
	if (result) {
		printf("create: vfs_walk(%s) failed %d\n", parent_path, result);
		return ERR_UNKNOWN;
	}

	vnode_t *vnode;
	result = vfs_create(parent, name, O_WRONLY,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
					| S_IROTH | S_IWOTH,
			permission, &vnode);
	vnodes_remove(parent);

	if (result) {
		printf("create: create(%s) failed %d\n", name, result);
		return ERR_UNKNOWN;
	}

	int fd = open(from, O_RDONLY);
	if (fd < 0) {
		printf("create: open(%s) failed %d\n", from, result);
		//TODO remove file
		vnodes_remove(vnode);
		return ERR_FILE;
	}

	char *buf = malloc(fs->device.block_size);
	if (!buf) {
		printf("create: malloc failed\n");
		//TODO remove file
		vnodes_remove(vnode);
		close(fd);
		return ERR_MEMORY;
	}

	//TODO use off_t
//	off_t offset = 0;
	size_t offset = 0;
	for (;;) {
		ssize_t len = read(fd, buf, fs->device.block_size);
		if (len == -1) {
			printf("create: read(%s) failed %d\n", from, errno);
			free(buf);
			//TODO remove file
			vnodes_remove(vnode);
			close(fd);
			return ERR_FILE;
		}

		if (len == 0)
			break;

		copier_t copier = {
			copy_from,
			buf
		};
		size_t rlength;
		int error_no = vfs_write(vnode, &copier, offset, len, &rlength);
		if (error_no) {
			printf("create: write(%d, %d) failed %d\n",
					vnode->index, offset, error_no);
			free(buf);
			//TODO remove file
			vnodes_remove(vnode);
			close(fd);
			return ERR_FILE;
		}

		if (rlength != len) {
			printf("create: write(%d, %d) partially failed\n",
					vnode->index, offset);
			free(buf);
			//TODO remove file
			vnodes_remove(vnode);
			close(fd);
			return ERR_FILE;
		}

		offset += rlength;
	}

	free(buf);
	close(fd);
	vnodes_remove(vnode);

	return 0;
}

static int do_remove(vfs_t *fs, char *path, const struct permission *permission)
{
	vnode_t *vnode;
	int result = vfs_walk(fs->root, path, O_ACCMODE, permission, &vnode);
	if (result) {
		printf("remove: vfs_walk(%s) failed %d\n", path, result);
		return ERR_UNKNOWN;
	}

	result = vfs_remove(vnode, permission);
	if (result) {
		printf("remove: remove(%s) failed %d\n", path, result);
		vnodes_remove(vnode);
		return ERR_UNKNOWN;
	}

	return 0;
}

static int do_mkdir(vfs_t *fs, char *path, const struct permission *permission)
{
	char *parent_path = "";
	char *name = vfs_split_path(path, &parent_path);
	vnode_t *parent;
	int result = vfs_walk(fs->root, parent_path, O_ACCMODE, permission,
			&parent);
	if (result) {
		printf("mkdir: vfs_walk(%s) failed %d\n", parent_path, result);
		return ERR_UNKNOWN;
	}

	vnode_t *vnode;
	result = vfs_create(parent, name, O_RDONLY,
			DMDIR | S_IRWXU | S_IRWXG | S_IRWXO,
			permission, &vnode);
	vnodes_remove(parent);

	if (result) {
		printf("mkdir: create(%s) failed %d\n", name, result);
		return ERR_UNKNOWN;
	}

	vnodes_remove(vnode);

	return 0;
}

static int do_chmod(vfs_t *fs, const char *mode, char *path,
		const struct permission *permission)
{
	vnode_t *vnode;
	//TODO use constant definition of guest
	int result = vfs_walk(fs->root, path, O_ACCMODE, permission, &vnode);
	if (result) {
		printf("chmod: vfs_walk(%s) failed %d\n", path, result);
		return ERR_UNKNOWN;
	}

	int m = strtol(mode, NULL, 8);
	if (m & UNMODIFIABLE_MODE_BITS) {
		printf("chmod: invalid mode %s\n", mode);
		vnodes_remove(vnode);
		return ERR_ARG;
	}

	//TODO use constant definition of guest
	vstat_t st;
	st.mode = m;
	st.gid = (uint32_t) (-1);
	st.size = -1;
	st.mtime = -1;
	result = vnode->fs->operations.wstat(vnode, &st);
	vnodes_remove(vnode);

	if (result) {
		printf("chmod: chmod(%s) failed %d\n", path, result);
		return ERR_UNKNOWN;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("no arguments\n");
		return ERR_ARG;
	}

	if (argc == 2) {
		printf("no block size\n");
		return ERR_ARG;
	}

	if (argc == 3) {
		printf("no commands\n");
		return ERR_ARG;
	}

	int result;
	result = initialize();
	if (result)
		return result;

	int block_size = atoi(argv[2]);
	result = mount(&fs, argv[1], block_size);
	if (result)
		return result;

	// manipulate vfs here
	if (argc > 3) {
		if (!strcmp(argv[3], "ls")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_ls(&fs, argv[4], &permission);
		} else if (!strcmp(argv[3], "create")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_create(&fs, argv[4], argv[5],
						&permission);
		} else if (!strcmp(argv[3], "remove")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_remove(&fs, argv[4], &permission);
		} else if (!strcmp(argv[3], "mkdir")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_mkdir(&fs, argv[4], &permission);
/*
		} else if (!strcmp(argv[3], "mv")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_mv(&fs, argv[4], argv[5],
						&permission);
*/
		} else if (!strcmp(argv[3], "chmod")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_chmod(&fs, argv[4], argv[5],
						&permission);

		} else {
			printf("unknown command %s\n", argv[3]);
			result = ERR_ARG;
		}

		if (result) {
			unmount(&fs);
			return result;
		}
	}

	result = unmount(&fs);
	if (result)
		return result;

	return 0;
}
