

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>

static struct hrtimer hr_timer;

s64 k = 1;
int init_module(void)
{	
	///s64 k = 1;
	hrtimer_start(&hr_timer, k*1E6L, HRTIMER_MODE_REL);
	return 0;
}
void cleanup_module(void)
{
	printk(KERN_INFO "By !\n");
	return;
}
MODULE_LICENSE("GPL");
