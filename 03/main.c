#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static int do_work(int time)
{
	int x;
	int z;

	for (x = 0; x < time; ++x)
		udelay(10);
	if (time > 10)
		pr_info("We slept a long time!");
	z = x * time;
	return z;
}

static __init int my_init(void)
{
	do_work(10);
	return 0;
}

static __exit void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
