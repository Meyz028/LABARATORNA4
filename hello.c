// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ktime.h>

MODULE_AUTHOR("Shmihel Makar <IO-44>");
MODULE_DESCRIPTION("Hello world module with parameter and event list");
MODULE_LICENSE("GPL");

struct hello_event {
	struct list_head list;
	ktime_t time;
};

static LIST_HEAD(hello_list);

static uint count = 1;
module_param(count, uint, 0444);
MODULE_PARM_DESC(count, "How many times to print \"Hello, world!\" (default 1, max 10)");

static void hello_free_list(void)
{
	struct hello_event *pos, *tmp;

	list_for_each_entry_safe(pos, tmp, &hello_list, list) {
		pr_info("Event time: %lld ns\n", ktime_to_ns(pos->time));
		list_del(&pos->list);
		kfree(pos);
	}
}

static int __init hello_init(void)
{
	struct hello_event *ev;
	uint i;

	if (count > 10) {
		pr_err("hello: count=%u is too big (max 10)\n", count);
		return -EINVAL;
	}

	if (count == 0 || (count >= 5 && count <= 10))
		pr_warn("hello: count=%u, continuing anyway\n", count);

	for (i = 0; i < count; i++) {
		ev = kmalloc(sizeof(*ev), GFP_KERNEL);
		if (!ev) {
			hello_free_list();
			return -ENOMEM;
		}
		ev->time = ktime_get();
		list_add_tail(&ev->list, &hello_list);
		pr_info("Hello, world!\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	hello_free_list();
}

module_init(hello_init);
module_exit(hello_exit);
