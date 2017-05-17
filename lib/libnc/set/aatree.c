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
#include <set/tree.h>
#include "aatree.h"

node_t nil_node = { 0, 0, &nil_node, &nil_node };

static void node_destroy(tree_t *, node_t *);
static node_t *node_skew(node_t *);
static node_t *node_split(node_t *);
static node_t *node_put(tree_t *, node_t *, node_t *);
static node_t *node_rebalance(node_t *);
static node_t *node_remove_swap(tree_t *, node_t *, node_t **);
static node_t *node_remove(tree_t *, node_t *, const int);


static void node_destroy(tree_t *tree, node_t *node)
{
	tree->removed = node;
	tree->node_num--;
}

static node_t *node_skew(node_t *p)
{
	if (p->left->level == p->level) {
		node_t *tmp = p;

		p = p->left;
		tmp->left = p->right;
		p->right = tmp;
	}

	return p;
}

static node_t *node_split(node_t *p)
{
	if (p->right->right->level == p->level) {
		node_t *tmp = p;

		p = p->right;
		tmp->right = p->left;
		p->left = tmp;
		p->level++;
	}

	return p;
}

static node_t *node_put(tree_t *tree, node_t *p, node_t *child)
{
	if (IS_NIL(p))
		p = child;
	else {
		int d = tree->compare(child->key, p->key);

		if (d == 0)
			return NULL;

		if (d < 0) {
			node_t *q = node_put(tree, p->left, child);

			if (!q)
				return NULL;

			p->left = q;
		} else {
			node_t *q = node_put(tree, p->right, child);

			if (!q)
				return NULL;

			p->right = q;
		}

		p = node_split(node_skew(p));
	}

	return p;
}

static node_t *node_rebalance(node_t *p)
{
	if ((p->left->level < (p->level - 1))
			|| (p->right->level < (p->level - 1))) {
		if (p->right->level > --p->level)
			p->right->level = p->level;

		p = node_skew(p);
		p->right = node_skew(p->right);
		p->right->right = node_skew(p->right->right);
		p = node_split(p);
		p->right = node_split(p->right);
	}

	return p;
}

static node_t *node_remove_swap(tree_t *tree, node_t *p, node_t **to)
{
	if (IS_NIL(p->left)) {
		node_t *q = p->right;

		p->level = (*to)->level;
		p->left = (*to)->left;
		node_destroy(tree, *to);
		*to = p;
		return q;
	} else
		p->left = node_remove_swap(tree, p->left, to);

	return  node_rebalance(p);
}

static node_t *node_remove(tree_t *tree, node_t *p, const int key)
{
	if (!IS_NIL(p)) {
		int d = tree->compare(key, p->key);

		if (d == 0) {
			if (IS_NIL(p->left)) {
				node_t *q = p->right;

				node_destroy(tree, p);
				return q;
			} else if (IS_NIL(p->right)) {
				node_t *q = p->left;

				node_destroy(tree, p);
				return q;
			} else {
				node_t *q = p;
				node_t *r = node_remove_swap(tree, p->right,
						&q);

				p = q;
				p->right = r;
			}
		} else if (d < 0)
			p->left = node_remove(tree, p->left, key);
		else
			p->right = node_remove(tree, p->right, key);

		p = node_rebalance(p);
	}

	return p;
}

void tree_create(tree_t *tree, int (*compare)(const int a, const int b))
{
	tree->compare = compare? compare:compare_int;
	tree->node_num = 0;
	tree->root = NIL;
}

node_t *tree_get(tree_t *tree, const int key)
{
	node_t *p = tree->root;

	while (!IS_NIL(p)) {
		int d = tree->compare(key, p->key);

		if (d == 0)
			return p;

		p = (d < 0)? p->left:p->right;
	}

	return NULL;
}

node_t *tree_put(tree_t *tree, const int key, node_t *node)
{
	node_t *p;

	node->key = key;
	node->level = 1;
	node->right = node->left = NIL;

	p = node_put(tree, tree->root, node);
	if (p) {
		tree->root = p;
		tree->node_num++;
	}

	return p;
}

node_t *tree_remove(tree_t *tree, const int key)
{
	tree->removed = NULL;
	tree->root = node_remove(tree, tree->root, key);
	return tree->removed;
}
