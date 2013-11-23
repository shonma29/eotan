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
#include <nerve/config.h>
#include <set/tree.h>


node_t *find_empty_key(tree_t *tree, int *hand)
{
	int key;
/*
	int max = (tree == &(queue_tree))? 49156:MAX_AUTO_ID;

	for (key = *hand + 1; key <= max; key++) {
*/
	for (key = *hand + 1; key <= MAX_AUTO_ID; key++) {
		if (!tree_get(tree, key)) {
			*hand = key;
			return tree_put(tree, key);
		}
	}
/* TODO test */
	for (key = MIN_AUTO_ID; key < *hand; key++) {
		if (!tree_get(tree, key)) {
			*hand = key;
			return tree_put(tree, key);
		}
	}

	return NULL;
}
