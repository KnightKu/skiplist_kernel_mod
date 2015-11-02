/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * GPL HEADER END
 */

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>

#define skiplist_for_each(pos, end) \
        for (; pos != end; pos = pos->next)

#define skiplist_for_each_safe(pos, n, end) \
        for (n = pos->next; pos != end; pos = n, n = pos->next)

#define MAX_LEVEL 32  /* Should be enough for 2^32 elements */

void get_random_bytes(void *buf, int nbytes);

struct skiplist {
	int level;
	int count;
	struct list_head head[MAX_LEVEL];
};

struct skipnode {
	int key;
	int value;
	struct list_head link[0];
};

static struct skipnode *
skipnode_create(int level, int key, int value)
{
	struct skipnode *node;

	node = kmalloc(sizeof(*node) + level * sizeof(struct list_head), GFP_KERNEL);
	if (!node)
		return ERR_PTR(-ENOMEM);

	node->key = key;
	node->value = value;
	return node;
}

static void
skipnode_destroy(struct skipnode *node)
{
	kfree(node);
}

static struct skiplist *
skiplist_create(void)
{
	int i;
	struct skiplist *list;

	list = kmalloc(sizeof(*list), GFP_KERNEL);
	if (!list)
		return ERR_PTR(-ENOMEM);

	list->level = 1;
	list->count = 0;

	for (i = 0; i < MAX_LEVEL; i++)
		INIT_LIST_HEAD(&list->head[i]);
	return list;
}

static void
skiplist_destroy(struct skiplist *list)
{
	struct list_head *pos, *n;

	pos = list->head[0].next;
	skiplist_for_each_safe(pos, n, &list->head[0]) {
		struct skipnode *node =
			list_entry(pos, struct skipnode, link[0]);

		skipnode_destroy(node);
        }

	kfree(list);
}

/* How to slelect the suitable level, random... */
static int
select_level(void)
{
	int level = 1;
	u16 rand = 0;

	while (1) {
		get_random_bytes(&rand, sizeof(rand));
		if ((rand & 0xffff) >= 0xffff >> 2)
			break;
		level++;
	}

	return level > MAX_LEVEL ? MAX_LEVEL : level;
}

static struct skipnode *
skiplist_search(struct skiplist *list, int key)
{
	int i = list->level - 1;
	struct list_head *pos = &list->head[i];
	struct list_head *end = &list->head[i];
	struct skipnode *node;

	for (; i >= 0; i--) {
		pos = pos->next;
		skiplist_for_each(pos, end) {
			node = list_entry(pos, struct skipnode, link[i]);
			if (node->key >= key) {
				end = &node->link[i];
				break;
			}
		}
		if (node->key == key) {
			return node;
		}
		pos = end->prev;
		pos--;
		end--;
	}

	return NULL;
}

static struct skipnode *
skiplist_search_first_eq_big(struct skiplist *list, int key)
{
	int i = list->level - 1;
	struct list_head *pos = &list->head[i];
	struct list_head *end = &list->head[i];
	struct skipnode *perf_node = NULL;

	for (; i >= 0; i--) {
		struct skipnode *node = NULL;

		pos = pos->next;
		skiplist_for_each(pos, end) {
			node = list_entry(pos, struct skipnode, link[i]);
			if (node->key >= key) {
				end = &node->link[i];
				perf_node = node;
				break;
			}
		}
		if (node && node->key == key) {
			return node;
		}
		pos = end->prev;
		pos--;
		end--;
	}
	return perf_node;
}

static struct skipnode *
skiplist_insert(struct skiplist *list, int key, int value)
{
	int level = select_level();
	int i;
	struct list_head *pos, *end;
        struct skipnode *node;

	if (level > list->level)
		list->level = level;


        node = skipnode_create(level, key, value);
	if (!node)
		return ERR_PTR(-ENOMEM);

	i = list->level - 1;
	pos = end = &list->head[i];

	for (; i >= 0; i--) {
		pos = pos->next;
		skiplist_for_each(pos, end) {
			struct skipnode *nd =
				list_entry(pos, struct skipnode, link[i]);

			if (nd->key >= key) {
				end = &nd->link[i];
				break;
			}
		}
		pos = end->prev;
		if (i < level)
			__list_add(&node->link[i], pos, end);
		pos--;
		end--;
	}

        list->count++;

	return node;
}

static void
skiplist_remove_node(struct skiplist *list, struct skipnode *node, int level)
{
	int i;

	for (i = 0; i < level; i++) {
		list_del(&node->link[i]);
		if (list_empty(&list->head[i]))
			list->level--;
        }
        skipnode_destroy(node);
        list->count--;
}

static void
skiplist_remove(struct skiplist *list, int key)
{
	int i = list->level - 1;
	struct list_head *pos = &list->head[i];
	struct list_head *end = &list->head[i];
	struct list_head *n;

	for (; i >= 0; i--) {
		pos = pos->next;
		skiplist_for_each_safe(pos, n, end) {
			struct skipnode *node =
				list_entry(pos, struct skipnode, link[i]);

			if (node->key == key) {
				skiplist_remove_node(list, node, i + 1);
			} else if (node->key > key) {
				end = &node->link[i];
				break;
			}
		}
		pos = end->prev;
		pos--;
		end--;
	}
}

static void
skiplist_dump(struct skiplist *list)
{
	int i = list->level - 1;
	struct list_head *pos = &list->head[i];
	struct list_head *end = &list->head[i];

	printk("\nTotal %d nodes: \n", list->count);
	for (; i >= 0; i--) {
		pos = pos->next;
		skiplist_for_each(pos, end) {
			struct skipnode *node =
				list_entry(pos, struct skipnode, link[i]);

			printk(KERN_DEBUG "level:%d key:%d value:%d\n",
			       i + 1, node->key, node->value);
		}
		pos = &list->head[i];
		pos--;
		end--;
	}
}

#endif  /* _SKIPLIST_H */
