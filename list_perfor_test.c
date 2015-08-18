#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hashtable.h>

#include "skiplist.h"

#define N 1000000

struct hlist_slot {

	void * value;
	union
	{
		struct hlist_node link;
		struct list_head list;
	};
};

DEFINE_HASHTABLE(h_list, 12);

LIST_HEAD(l_head);

static inline struct hlist_slot *alloc_hlist_slot(void)
{
	struct hlist_slot * slot = kmalloc(sizeof(struct hlist_slot), GFP_KERNEL);

	return slot;
}

static inline void free_hlist_slot(struct hlist_slot *hlist_slot)
{
	kfree(hlist_slot);
}

static struct hlist_slot *get_hlist_slot(void *value)
{
	struct hlist_head *hhead = &h_list[hash_min((unsigned long)value, HASH_BITS(h_list))];
	struct hlist_node *node;
	struct hlist_slot *slot;

	hlist_for_each_entry(slot, node, hhead, link)
		if (slot->value == value)
			return slot;

	return NULL;
}

static void insert_slots_hash(struct hlist_slot *hlist_slot, void *value)
{
	hlist_slot->value = value;
	hash_add(h_list, &hlist_slot->link, (unsigned long)hlist_slot->value);
}

static void hashtable_test(void)
{
	unsigned long i = 0;
	struct hlist_slot *slot;
	struct timeval start, end;

	printk("Hashtable Test start!\n");
	printk("Add %d nodes...\n", N);

	do_gettimeofday(&start);
	while (i < N) {
		slot = alloc_hlist_slot();

		insert_slots_hash(slot, (void *)i);
		i++;
	}
	do_gettimeofday(&end);
        printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

	/* Search test */
	printk("Now search each node...\n");
	do_gettimeofday(&start);
	i = 0;
	while (i < N) {
		slot = get_hlist_slot((void *)i);
		if (!slot)
			printk(KERN_ERR "Key: %lu not found! hash table broken!\n", i);
		i ++;
	}

	do_gettimeofday(&end);
	printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

	/* Delete test */
	printk("Now remove all nodes...\n");
	do_gettimeofday(&start);
	for (i = 0; i < (1 << 12); i++) {
		struct hlist_head *hhead = &h_list[i];
		struct hlist_node *node, *next;

		hlist_for_each_entry_safe(slot, node, next, hhead, link) {
			hash_del(&slot->link);
			free_hlist_slot(slot);
		}
	}
	do_gettimeofday(&end);
	printk("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
}

static void list_test(void)
{
	unsigned long i = 0;
	struct hlist_slot *slot, *tmp;
	struct timeval start, end;

	printk("Test start!\n");
	printk("Add %d nodes...\n", N);
	while (i < N) {
		slot = alloc_hlist_slot();

		slot->value = (void *)i;
		list_add_tail(&slot->list, &l_head);
		i ++;
	}
	do_gettimeofday(&end);
	printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

	i = 0;
	/* Search test */
	printk("Now search each node...\n");
	do_gettimeofday(&start);
	while (i < N) {
		bool get = false;

		list_for_each_entry(slot, &l_head, list)
			if (slot->value == (void *)i) {
				get = true;
				break;
			}

		if (unlikely(!get))
			printk(KERN_ERR "Key: %lu not found! hash table broken!\n", i);
		i++;
	}
	do_gettimeofday(&end);
	printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

	/* Delete test */
	printk("Now remove all nodes...\n");
	do_gettimeofday(&start);
	list_for_each_entry_safe(slot, tmp, &l_head, list) {
		list_del(&slot->list);
		free_hlist_slot(slot);
	}
        do_gettimeofday(&end);
        printk("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
}

static void skip_list_test(void)
{
	int i;
	struct timeval start, end;
	struct skiplist *list;

	int *key = kmalloc(N * sizeof(int), GFP_KERNEL);
	if (key == NULL) {
		printk("-ENOMEM\n");
		return;
	}

	list = skiplist_create();
        if (list == NULL) {
		printk("-ENOMEM\n");
		return;
        }

	printk("Test start!\n");
	printk("Add %d nodes...\n", N);

	do_gettimeofday(&start);
	for (i = 0; i < N; i++) {
		int value = key[i] = i;

		skiplist_insert(list, key[i], value);
	}

	do_gettimeofday(&end);
	printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
#ifdef SKIPLIST_DEBUG
	skiplist_dump(list);
#endif

	/* Search test */
	printk("Now search each node...\n");
	do_gettimeofday(&start);
	for (i = 0; i < N; i++) {
		struct skipnode *node = skiplist_search(list, key[i]);

		if (node != NULL) {
#ifdef SKIPLIST_DEBUG
			printk("key:0x%08x value:0x%08x\n", node->key, node->value);
#endif
		} else {
			printk("Not found:0x%08x\n", key[i]);
		}
	}
	do_gettimeofday(&end);
	printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

	/* Delete test */
	printk("Now remove all nodes...\n");
	do_gettimeofday(&start);

	for (i = 0; i < N; i++)
		skiplist_remove(list, key[i]);

	do_gettimeofday(&end);
	printk("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
#ifdef SKIPLIST_DEBUG
	skiplist_dump(list);
#endif
	printk("End of Test.\n");
	skiplist_destroy(list);
}

static int __init list_perfor_test_init(void)
{
	printk("==========testing start=========\n");
	printk("\n\n\n");
	printk(">>>>>>>>skip-list testing=========\n");
	skip_list_test();
	printk(">>>>>>>>list testing=========\n");
	list_test();
	printk(">>>>>>>>hash table testing=========\n");
	hashtable_test();
	printk("==========testing over=========\n");
	return 0;
}

static void __exit list_perfor_test_exit(void)
{
	printk("==========exit testing mod=========\n");
}

module_init(list_perfor_test_init);
module_exit(list_perfor_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gu Zheng <gzheng@ddn.com>");
MODULE_DESCRIPTION("Skip-list, list, Hash-table Performance test");
