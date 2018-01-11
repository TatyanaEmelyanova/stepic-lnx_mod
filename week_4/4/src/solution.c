/*
 * Разработайте модуль ядра solution, реализующий настройку и запуск нескольких таймеров по цепочке.
 * Задержка запуска каждого таймера передается в модуль в виде параметра-массива (unsigned long delays[]) в миллисекундах. 
 * Задержка для таймера номер N (начиная с нуля) в цепочке должна соответствовать N-ому значению массива delays. 
 * Каждая задержка может составлять от 50 мс до 1 с. При запуске первого таймера, а так же при каждом очередном 
 * срабатывании обработчика таймера в целях проверки необходимо вызывать функцию check_timer из модуля checker.
 *
 * Пример передачи массива с задержками в модуль:
 *
 * sudo insmod solution.ko delays=150,500,850,50,1000,350,200 
 *
 * Прототип функции для вызова:
 *
 * void check_timer(void); 
 *
 * Прототип задается в файле checker.h .
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
//
#define MAX_DELAYS_SIZE 1024
static int delays[MAX_DELAYS_SIZE];
static int delays_cnt = 0;


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

#define MS_TO_NS(x) (x * 1E6L)
module_param_array(delays, int, &delays_cnt, 0);

static struct hrtimer hr_timer;
static ktime_t delay_ns;
static int cur_delay_cnt = 0;


static enum hrtimer_restart timer_callback(struct hrtimer *timer){

	//printk( KERN_INFO "kernel_mooc %s : cur_delay_cnt = %d \n", __FUNCTION__, cur_delay_cnt);
	//check_timer();
	if ( cur_delay_cnt >= delays_cnt ) return HRTIMER_NORESTART;
 	
 	delay_ns = ktime_set( 0, delays[cur_delay_cnt++] * NSEC_PER_MSEC);
	hrtimer_forward_now(&hr_timer, delay_ns);
	return HRTIMER_RESTART;
}

s64 k = 1;
void test(void)
{
	//s64 k = 1;
	hrtimer_start(&hr_timer, k*1E1L, HRTIMER_MODE_REL);
	return;
}

//hrtimer_start(&hr_timer, k*10L, HRTIMER_MODE_REL);

int slt_init(void)
{

	
	if ( delays_cnt <= 0 ) return 0;
	//delay_ns = ktime_set( 0, delays[cur_delay_cnt++] * NSEC_PER_MSEC);
    hrtimer_init ( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    hr_timer.function = timer_callback;
    //check_timer();
    
    hrtimer_start( &hr_timer, ktime_set( 0, (delays[cur_delay_cnt++] * 1000000L)), HRTIMER_MODE_REL );
	
	//printk( KERN_INFO "kernel_mooc %s : ========Start Timer : delays_cnt = %d ===========\n", __FUNCTION__, delays_cnt );
	return 0;
}

void slt_exit(void)
{
	hrtimer_cancel(&hr_timer);
	return;
}

module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");