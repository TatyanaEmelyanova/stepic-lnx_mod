#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void)
{
	printk(KERN_INFO "Hi !\n");
	return 0;
}
void cleanup_module(void)
{
	printk(KERN_INFO "By !\n");
	return;
}
MODULE_LICENSE("GPL");


