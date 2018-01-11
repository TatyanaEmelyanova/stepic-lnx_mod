/*
Разработать модуль ядра solution, который принимает на вход следующие параметры:

a типа int
b типа int
c массив из 5ти элементов типа int

Модуль считает сумму a+b+c[@], создает kobject с именем /sys/kernel/my_kobject/my_sys 
и выводит в него результат вычисления.


sudo /solution.ko a=3 b=4 c=5,4,3,2,1

sys/kernel/my_kobject/my_sys -> 22
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#define C_SIZE 5
static int a;
static int b;
static int c[C_SIZE];

/*
 * module_param - typesafe helper for a module/cmdline parameter
 * @value: the variable to alter, and exposed parameter name.
 * @type: the type of the parameter
 * @perm: visibility in sysfs.

 * #define module_param(name, type, perm)				\
 *	module_param_named(name, name, type, perm)
 */

module_param(a, int, 0);
module_param(b, int, 0);

/**
 * module_param_array - a parameter which is an array of some type
 * @name: the name of the array variable
 * @type: the type, as per module_param()
 * @nump: optional pointer filled in with the number written
 * @perm: visibility in sysfs
 *
 * Input and output are as comma-separated values.  Commas inside values
 * don't work properly (eg. an array of charp).
 *
 * ARRAY_SIZE(@name) is used to determine the number of elements in the
 * array, so the definition must be visible.
 
 * #define module_param_array(name, type, nump, perm)		\
 *  module_param_array_named(name, name, type, nump, perm)

*/

module_param_array(c, int, NULL, 0);


static ssize_t cnt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int res =  a + b;
	int i;
	for(i = 0; i < C_SIZE; ++i) res += c[i];

	return sprintf(buf, "%d\n", res);
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