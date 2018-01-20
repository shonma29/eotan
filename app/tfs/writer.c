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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fs/vfs.h>

#define ERR_ARG (1)
#define ERR_MEMORY (2)
#define ERR_FILE (3)
#define ERR_UNKNOWN (4)

typedef struct {
	long long int sec;
	long nsec;
} systime_t;

static vfs_t fs;
static vnode_t *root_node;

extern vfs_operation_t sfs_fsops;


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

	printf("initialize: root_node = %p\n", *root);

	return 0;
}

static int mount(vfs_t *fs, const char *filename, vnode_t *root)
{
	int fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("file open failed %d\n", errno);
		return ERR_FILE;
	}

	list_initialize(&(fs->bros));
	list_initialize(&(fs->vnodes));

	int result = sfs_fsops.mount(fd, fs, root);
	if (result) {
		printf("mount failed %d\n", result);
		close(fd);
		return ERR_UNKNOWN;
	}

	fs->root = root;
	fs->operations = sfs_fsops;
	root->fs = fs;
	vnodes_append(root);

	printf("mount: fd = %d\n", fd);

	return 0;
}

static int unmount(vfs_t *fs, vnode_t *root)
{
	int result = sfs_fsops.unmount(fs);
	if (result) {
		printf("unmount failed %d\n", result);
		close(fs->device.channel);
		return ERR_UNKNOWN;
	}

	result = vnodes_remove(root);
	if (result) {
		printf("vnodes_remove failed %d\n", result);
		close(fs->device.channel);
		return ERR_UNKNOWN;
	}

	if (close(fs->device.channel)) {
		printf("file close failed %d\n", errno);
		return ERR_FILE;
	}

	printf("unmount: done = %d\n", fs->device.channel);

	return 0;
}

int main(int argc, char **argv)
{
	int result;

	result = initialize(&root_node);
	if (result)
		return result;

	if (argc <= 1) {
		printf("no arguments\n");
		vnodes_remove(root_node);
		return ERR_ARG;
	}

	result = mount(&fs, argv[1], root_node);
	if (result) {
		vnodes_remove(root_node);
		return result;
	}

	// manipulate vfs here

	result = unmount(&fs, root_node);
	if (result)
		return result;

	return 0;
}
