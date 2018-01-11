/*
 * Необходимо создать модуль ядра, реализующий обработчик прерываний для устройства Real Time Clock, использующего IRQ №8. 
 * Обработчик прерываний должен вести подсчет прерываний и отображать его в kobject /sys/kernel/my_kobject/my_sys.
 * 
 * Для самостоятельного тестирования рекомендуется использовать команду
 * 
 * sudo bash -c ' echo +20 > /sys/class/rtc/rtc0/wakealarm '
 * 
 * которая осуществляет установку таймера для RTC и файл /proc/interrupts для подсчета числа прерываний.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>

static int irq_cnt = 0;
static int  my_dev_id;
static const int IRQ_NUM = 8;

static irqreturn_t my_interrupt( int irq, void *dev_id )
{
	irq_cnt++;
	printk( KERN_INFO "kernel_mooc %s : In the ISR: counter = %d\n", __FUNCTION__, irq_cnt );
	return IRQ_NONE; /* we return IRQ_NONE because we are just observing */
}


static ssize_t cnt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{

	return sprintf(buf, "%d\n", irq_cnt);
}

static struct kobj_attribute cnt_attr = __ATTR(my_sys, 0664, cnt_show, NULL);

static struct attribute * attrs[] = { &cnt_attr.attr, NULL };

static struct attribute_group attr_group = { .attrs = attrs };

static struct kobject *my_kobject;

int slt_init(void)
{
	int retval;

	if ( request_irq( IRQ_NUM, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id ) ) 
		return -1;
	printk( KERN_INFO "kernel_mooc %s : Successfully loading ISR handler on IRQ %d\n", __FUNCTION__, IRQ_NUM );

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
	synchronize_irq( IRQ_NUM );
	free_irq( IRQ_NUM, &my_dev_id );
	printk( KERN_INFO "kernel_mooc %s : Successfully unloading, irq_counter = %d\n", __FUNCTION__, irq_cnt );

	return;
}

module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");