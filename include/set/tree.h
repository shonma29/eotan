#ifndef TREE_H
#define TREE_H
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
#include "stddef.h"
#include "set/slab.h"

typedef struct _node_t {
	int key;
	int level;
	struct _node_t *left;
	struct _node_t *right;
} node_t;

typedef struct _tree_t {
	size_t node_num;
	node_t *root;
	slab_t *slab;
} tree_t;

#define tree_max_block(max_unit, page_size, unit_size) ((max_unit) \
		/ (((page_size) - sizeof(slab_block_t)) \
				/ (sizeof(node_t) + (unit_size))))


extern void tree_create(tree_t *tree, slab_t *slab);
extern size_t tree_size(tree_t *tree);
extern node_t *tree_get(tree_t *tree, const int key);
extern node_t *tree_put(tree_t *tree, const int key);
extern void tree_remove(tree_t *tree, const int key);

#endif