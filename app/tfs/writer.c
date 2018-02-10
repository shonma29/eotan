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
#include "../../include/fs/vfs.h"
#include "libserv.h"
#include "writer.h"

#define ERR_ARG (1)
#define ERR_MEMORY (2)
#define ERR_FILE (3)
#define ERR_UNKNOWN (4)

//TODO fix another way
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
static vnode_t *root_node;
static struct permission permission = { 0, 0 };


void *palloc(void)
{
	return malloc(4096);
}

void pfree(void *p)
{
	free(p);
}

int dbg_printf(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	return vprintf(format, ap);
}

int time_get(systime_t *p)
{
	p->sec = time(NULL);
	p->nsec = 0;

	return 0;
}

static int initialize(vnode_t **root)
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

	*root = vnodes_create();
	if (!(*root)) {
		printf("vnodes_create failed\n");
		return ERR_MEMORY;
	}

//	printf("initialize: root_node = %p\n", *root);

	return 0;
}

static int mount(vfs_t *fs, const char *filename, vnode_t *root)
{
	int fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("file open(%s) failed %d\n", filename, errno);
		return ERR_FILE;
	}

	list_initialize(&(fs->bros));
	list_initialize(&(fs->vnodes));
	fs->operations = fsops;

	int result = fs->operations.mount(fd, fs, root);
	if (result) {
		printf("mount(%d) failed %d\n", fd, result);
		close(fd);
		return ERR_UNKNOWN;
	}

	fs->root = root;
	root->fs = fs;
	vnodes_append(root);

//	printf("mount: fd = %d\n", fd);

	return 0;
}

static int unmount(vfs_t *fs, vnode_t *root)
{
	int result = fs->operations.unmount(fs);
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

static int do_stat(vnode_t *ip, const char *path)
{
	//TODO use struct definition of guest
	vstat_t st;
	int result = ip->fs->operations.stat(ip, &st);

	if (result) {
		printf("stat(%d) failed %d\n", ip->index, result);
		return ERR_UNKNOWN;
	}

	format(&st, path);

	return 0;
}

static int readdir(vfs_t *fs, vnode_t *ip, const struct permission *permission)
{
	vdirent_t *buf = malloc(sizeof(vdirent_t));
	if (!buf) {
		printf("readdir: malloc failed\n");
		return ERR_MEMORY;
	}

	//TODO use off_t
//	off_t offset = 0;
	size_t offset = 0;
	for (;;) {
		size_t rlength;
		int error_no = fs->operations.getdents(ip, &offset, buf,
				&rlength);
		if (error_no) {
			printf("readdir: getdents(%d, %d) failed %d\n",
					ip->index, offset, error_no);
			free(buf);
			return ERR_FILE;
		}

		if (!rlength)
			break;

		vnode_t *entry;
		//TODO use constant definition of guest
		int result = vfs_walk(ip, buf->d_name, O_RDONLY,
				permission, &entry);
		if (result) {
			printf("readdir: vfs_walk(%s) failed %d\n",
					buf->d_name, result);
			free(buf);
			return ERR_UNKNOWN;
		}

		result = do_stat(entry, buf->d_name);
		vnodes_remove(entry);

		if (result) {
			printf("readdir: stat(%s) failed %d\n",
					buf->d_name, result);
			free(buf);
			return result;
		}
	}

	free(buf);

	return 0;
}

static int do_ls(vfs_t *fs, const char *path,
		const struct permission *permission)
{
	vnode_t *ip;

	//TODO use constant definition of guest in vfs
	int result = vfs_walk(fs->root, path, O_RDONLY, permission, &ip);
	if (result) {
		printf("ls: vfs_walk(%s) failed %d\n", path, result);
		return ERR_UNKNOWN;
	}

	//TODO use constant definition of guest, or define method in vnodes
	if ((ip->mode & S_IFMT) == S_IFDIR) {
//		printf("%s is directory\n", path);
//		do_stat(ip, path);
//		printf("---\n");
		result = readdir(fs, ip, permission);
		vnodes_remove(ip);

		if (result) {
			printf("ls: readdir(%d) failed %d\n",
					ip->index, result);
			return result;
		}

	} else {
		result = do_stat(ip, path);
		vnodes_remove(ip);

		if (result) {
			printf("ls: stat(%s) failed %d\n", path, result);
			return result;
		}
	}

	return 0;
}

static int do_create(vfs_t *fs, char *path, const char *from,
		const struct permission *permission)
{
	vnode_t *ip;
	int result = vfs_create(fs->root, path,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
					| S_IROTH | S_IWOTH,
			permission, &ip);
	if (result) {
		printf("create: create(%s) failed %d\n", path, result);
		return result;
	}

	int fd = open(from, O_RDONLY);
	if (fd < 0) {
		printf("create: open(%s) failed %d\n", from, result);
		//TODO remove file
		vnodes_remove(ip);
		return ERR_FILE;
	}

	char *buf = malloc(fs->device.block_size);
	if (!buf) {
		printf("create: malloc failed\n");
		//TODO remove file
		vnodes_remove(ip);
		close(fd);
		return ERR_MEMORY;
	}

	//TODO use off_t
//	off_t offset = 0;
	size_t offset = 0;
	for (;;) {
		ssize_t len = read(fd, buf, fs->device.block_size);
		if (len < 0) {
			printf("create: read(%s) failed\n", from, errno);
			free(buf);
			//TODO remove file
			vnodes_remove(ip);
			close(fd);
			return ERR_FILE;
		}

		if (len == 0)
			break;

		int rlength;
		int error_no = fs->operations.write(ip, buf, offset,
				len, &rlength);
		if (error_no) {
			printf("create: write(%d, %d) failed %d\n",
					ip->index, offset, error_no);
			free(buf);
			//TODO remove file
			vnodes_remove(ip);
			close(fd);
			return ERR_FILE;
		}

		if (rlength != len) {
			printf("create: write(%d, %d) partially failed\n",
					ip->index, offset);
			free(buf);
			//TODO remove file
			vnodes_remove(ip);
			close(fd);
			return ERR_FILE;
		}

		offset += rlength;
	}

	free(buf);
	close(fd);
	vnodes_remove(ip);

	return 0;
}

static int do_remove(vfs_t *fs, char *path, const struct permission *permission)
{
	int result = vfs_remove(fs->root, path, permission);
	if (result) {
		printf("remove: remove(%s) failed %d\n", path, result);
		return result;
	}

	return 0;
}

static int do_mkdir(vfs_t *fs, char *path, const struct permission *permission)
{
	vnode_t *ip;
	int result = vfs_mkdir(fs->root, path, S_IRWXU | S_IRWXG | S_IRWXO,
			permission, &ip);
	if (result) {
		printf("mkdir: mkdir(%s) failed %d\n", path, result);
		return result;
	}

	vnodes_remove(ip);

	return 0;
}

static int do_rmdir(vfs_t *fs, char *path, const struct permission *permission)
{
	int result = vfs_rmdir(fs->root, path, permission);
	if (result) {
		printf("rmdir: rmdir(%s) failed %d\n", path, result);
		return result;
	}

	return 0;
}

static int do_chmod(vfs_t *fs, const char *mode, char *path,
		const struct permission *permission)
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

	vnode_t *ip;
	//TODO use constant definition of guest
	//TODO is O_WRONLY correct?
	int result = vfs_walk(fs->root, head, O_RDWR, permission, &ip);
	if (result) {
		printf("chmod: vfs_walk(%s) failed %d\n", head, result);
		return ERR_UNKNOWN;
	}

	//TODO use constant definition of guest
	ip->mode = (ip->mode & S_IFMT) | strtol(mode, NULL, 8);
	result = ip->fs->operations.wstat(ip);
	vnodes_remove(ip);

	if (result) {
		printf("chmod: chmod(%s) failed %d\n", head, result);
		return result;
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
		printf("no commands\n");
		return ERR_ARG;
	}

	int result;
	result = initialize(&root_node);
	if (result)
		return result;

	result = mount(&fs, argv[1], root_node);
	if (result) {
		vnodes_remove(root_node);
		return result;
	}

	// manipulate vfs here
	if (argc > 2) {
		if (!strcmp(argv[2], "ls")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_ls(&fs, argv[3], &permission);
		} else if (!strcmp(argv[2], "create")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_create(&fs, argv[3], argv[4],
						&permission);
		} else if (!strcmp(argv[2], "remove")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_remove(&fs, argv[3], &permission);
		} else if (!strcmp(argv[2], "mkdir")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_mkdir(&fs, argv[3], &permission);
		} else if (!strcmp(argv[2], "rmdir")) {
			if (argc < 4) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_rmdir(&fs, argv[3], &permission);
/*
		} else if (!strcmp(argv[2], "mv")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_mv(&fs, argv[3], argv[4],
						&permission);
*/
		} else if (!strcmp(argv[2], "chmod")) {
			if (argc < 5) {
				printf("no parameter\n");
				result = ERR_ARG;
			} else
				result = do_chmod(&fs, argv[3], argv[4],
						&permission);

		} else {
			printf("unknown command %s\n", argv[2]);
			result = ERR_ARG;
		}

		if (result) {
			unmount(&fs, root_node);
			return result;
		}
	}

	result = unmount(&fs, root_node);
	if (result)
		return result;

	return 0;
}
