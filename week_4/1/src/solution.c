/*
 * Разработать модуль ядра solution, который обращается к связному 
 * списку структуры struct module из заголовочного
 * файла linux/module.h и выводит имена всех модулей в kobject 
 * с именем /sys/kernel/my_kobject/my_sys. 
 * 
 * Имена модулей должны выводиться в алфавитном порядке, каждое на новой строке.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/list_sort.h>

/*
 * - The buffer will always be PAGE_SIZE bytes in length. On i386, this
 * is 4096. 
 * 
 * - show() methods should return the number of bytes printed into the
 *    buffer. This is the return value of scnprintf().
 * 
 * - show() must not use snprintf() when formatting the value to be
 *    returned to user space. If you can guarantee that an overflow
 *    will never happen you can use sprintf() otherwise you must use
 *    scnprintf().
*/

struct mod_name_struct {
	struct list_head list;
	char name[MODULE_NAME_LEN]; 
};

LIST_HEAD(mn_list_head) ;

static ssize_t cnt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	struct mod_name_struct *cur_mod = NULL;

	int char_cnt = 0;
	list_for_each_entry(cur_mod , &mn_list_head, list) 
		char_cnt += scnprintf((buf + char_cnt), PAGE_SIZE - char_cnt, "%s\n", cur_mod->name);

	return char_cnt;
}

static struct kobj_attribute cnt_attr = __ATTR(my_sys, 0664, cnt_show, NULL);

static struct attribute * attrs[] = { &cnt_attr.attr, NULL };

static struct attribute_group attr_group = { .attrs = attrs };

static struct kobject *my_kobject;

/*
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "merge sort", which has O(nlog(n))
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
int cmp_str (void *priv, struct list_head *a, struct list_head *b) {

	char * s_a = list_entry(a, struct mod_name_struct, list)->name;
	char * s_b = list_entry(b, struct mod_name_struct, list)->name;

	while ((*s_a != 0) || (*s_b != 0)) {
		if (*s_a < *s_b) return -1;
		if (*s_a > *s_b) return  1;
		s_a++;
		s_b++;
	}
	if ((*s_a == 0) && (*s_b == 0)) return 0;
	if  (*s_a == 0) return -1;
	return 1;
}

void create_mn_list(void)
{
	struct module *cur_mod = NULL;
	struct mod_name_struct *new_mod = NULL;

	list_for_each_entry(cur_mod , (THIS_MODULE->list.prev), list) { 

		new_mod = kmalloc(sizeof(*new_mod), GFP_KERNEL);
		memcpy(new_mod->name, cur_mod->name, MODULE_NAME_LEN);

		INIT_LIST_HEAD(&new_mod->list);
		list_add_tail(&new_mod->list, &mn_list_head);; 
	}

	list_sort(NULL, &mn_list_head, cmp_str);
}

int slt_init(void)
{
	int retval;
	
	my_kobject = kobject_create_and_add("my_kobject", kernel_kobj);
	if (!my_kobject) {
		return -ENOMEM;
	}

	retval = sysfs_create_group( my_kobject, &attr_group );
	if (retval) kobject_put(my_kobject);
	
	create_mn_list();
	
	return retval;
}

void slt_exit(void)
{	
	struct mod_name_struct *cur_mod, *next_mod;

	kobject_put(my_kobject);

	list_for_each_entry_safe(cur_mod, next_mod, &mn_list_head, list) {
		list_del(&cur_mod->list);
		kfree(cur_mod);
	}
	
	printk(KERN_INFO "kernel_mooc %s : Successfully unloading\n", __FUNCTION__);

	return;
}

module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");