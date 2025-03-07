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
#include <services.h>
#include "mm.h"
#include "device.h"

static tree_t device_tree;
static mm_device_t devices[] = {
	{ { (int) 'c' }, PORT_CONSOLE },
	{ { (int) 'i' }, PORT_WINDOW },
	{ { (int) 's' }, PORT_FS }
};

#define NUM_OF_DEVICES (sizeof(devices) / sizeof(devices[0]))


int device_initialize(void)
{
	tree_create(&device_tree, NULL, NULL);

	for (int i = 0; i < NUM_OF_DEVICES; i++) {
		int name = devices[i].node.key;
		if (!tree_put(&device_tree, name, &(devices[i].node))) {
			log_err(MYNAME ": failed to register %c\n", name);
			return (-1);
		}
	}

	return 0;
}

mm_device_t *device_get(const int key)
{
	node_t *node = tree_get(&device_tree, key);
	return (node ? (mm_device_t *) getParent(mm_device_t, node) : NULL);
}
