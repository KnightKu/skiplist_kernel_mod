/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */
#include "skiplist.h"

#define N 1000000
// #define SKIPLIST_DEBUG

static int __init skiplist_init(void)
{
	int i;
	struct timeval start, end;
	struct skiplist *list;

	int *key = kmalloc(N * sizeof(int), GFP_KERNEL);
	if (key == NULL) {
		printk("-ENOMEM\n");
		return -1;
	}


	list = skiplist_create();
        if (list == NULL) {
		printk("-ENOMEM\n");
		return -1;
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
        for (i = 0; i < N; i++) {
                skiplist_remove(list, key[i]);
        }
        do_gettimeofday(&end);
        printk("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
#ifdef SKIPLIST_DEBUG
        skiplist_dump(list);
#endif

        printk("End of Test.\n");
        skiplist_destroy(list);

        return 0;  
}
static void __exit skiplist_exit(void)
{
    printk("remove\n");
}

module_init(skiplist_init);
module_exit(skiplist_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gu Zheng <cengku@gmail.com>");
MODULE_DESCRIPTION("Skip hist test");
