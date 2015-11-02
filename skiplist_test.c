#include "skiplist.h"

#define N 100
//#define SKIPLIST_DEBUG

static int __init skiplist_init(void)
{
	int i;
	struct timeval start, end;
	struct skiplist *list;
	struct skipnode *node;
	int res = 0;

	int *key = kmalloc(N * sizeof(int), GFP_KERNEL);
	if (key == NULL) {
		printk("-ENOMEM\n");
		return -1;
	}

	printk("Starting initialization...\n");
	list = skiplist_create();
        if (list == NULL) {
		printk("-ENOMEM\n");
		return -1;
        }

	printk("Started initialization...\n");
        printk("Test start!\n");

	/* Test 01 */ 
	printk("Test 01: adding and search %d nodes testing!\n", N);
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
        printk("Now search %d node...\n", N);
        do_gettimeofday(&start);

        for (i = 0; i < N; i++) {
                struct skipnode *node = skiplist_search(list, key[i]);

                if (node != NULL) {
#ifdef SKIPLIST_DEBUG
                        printk("key:%d value:%d\n", node->key, node->value);
#endif
                } else {
                        printk("Not found:%d\n", key[i]);
			res = 1;
			break;
                }
        }
        do_gettimeofday(&end);
        printk("time span:% ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
	if (res) {
		printk("Test 01: failed!\n");
		goto out_clean;
	} else {
		printk("Test 01: success!\n");
	}

	/* Test 02 */

	printk("Test 02: search single node (%d/2) testing!\n", N);
	node = skiplist_search(list, N/2);
	if (node && node->value == N/2) {
		printk("Test 02: Success!\n");
	} else {
		printk("Test 02: Failed!\n");
		res = 1;
		goto out_clean;
	}

	/* Test 03 */

	printk("Test 03: remove single node (%d/2) testing!\n", N);
	skiplist_remove(list, N/2);
	node = skiplist_search(list, N/2);
	if (!node) {
		printk("Test 03: Success!\n");
	} else {
		printk("Test 03: Failed (key:%d)!\n", node->key);
		res = 1;
		goto out_clean;
	}

	/* Test 04 */

	printk("Test 04: search single node equal or great than (%d/2) testing!\n", N);
	printk("Test 04: case 1: no equal node (%d/2) \n", N);
	node = skiplist_search_first_eq_big(list, N/2);
	if (!node || node->value != (N/2 + 1)) {
		printk("Test 04: Failed!\n");
		res = 1;
		goto out_clean;
	}

	printk("Test 04: case 2: has equal node (%d/2 + 1) \n", N);
	node = skiplist_search_first_eq_big(list, N/2 + 1);
	if (node && node->value == (N/2 + 1)) {
		printk("Test 04: Success!\n");
	} else {
		printk("Test 04: Failed!\n");
		res = 1;
		goto out_clean;
	}

	/* Test 05 */
	res = 0;
	printk("Test 05: remove all nodes\n");
        for (i = 0; i < N; i++) {
                skiplist_remove(list, key[i]);
        }

        for (i = 0; i < N; i++) {
		node = skiplist_search(list, key[i]);
		if (node) {
			res = 1;
			break;
		}
	}

	if (res)
		printk("Test 05: Failed!\n");
	else
		printk("Test 05: Success!\n");

#ifdef SKIPLIST_DEBUG
        skiplist_dump(list);
#endif

        printk("End of Test.\n");
out_clean:	

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
