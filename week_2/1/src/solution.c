/*
Разработать модуль ядра solution, который создает kobject с именем /sys/kernel/my_kobject/my_sys. 
В этом объекте должно отображаться актуальное число операций чтения, адресованных к данному kobject.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>



static ssize_t cnt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	static int cnt = 0;

	return sprintf(buf, "%d\n", ++cnt);
}

static struct kobj_attribute cnt_attr = __ATTR(my_sys, 0664, cnt_show, NULL);

static struct attribute * attrs[] = { &cnt_attr.attr, NULL };

static struct attribute_group attr_group = { .attrs = attrs };

static struct kobject *my_kobject;

int slt_init(void)
{
	int retval;

	my_kobject = kobject_create_and_add("my_kobject" , kernel_kobj);
	if(!my_kobject) {
		return -ENOMEM;
	}

	retval = sysfs_create_group(my_kobject, &attr_group);
	if(retval) kobject_put(my_kobject);
	
	return retval;
}

void slt_exit(void)
{
	
	kobject_put(my_kobject);
	return;
}


module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");