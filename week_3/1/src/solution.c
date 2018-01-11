
/* 
 * Разработать модуль ядра solution, который реализует файловый интерфейс драйвера символьного устройства. 
 * В качестве устройства необходимо использовать node, задаваемый через строковый параметр модуля node_name.
 * Драйвер должен сам создать node по заданному имени в /dev и получить для него major number.
 * Полученное значение необходимо предоставлять пользователям при попытках чтения из устройства.

 * Пример загрузки модуля:

 * sudo insmod solution.ko node_name="my_super_cool_but_random_node_name"
 * Результаты работы драйвера должны быть доступны при чтении с символьного устройства в следующем формате

 * MAJOR\n
 * где MAJOR - это major number, динамически полученный модулем.
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>


//#define MOD_NAME "solution_node"
//static const int major = 240;
//static const int minor = 0;
static char * MOD_NAME = "my_super_cool_but_random_node_name";
static int major;
static int minor;

static size_t cnt_bytes_wr = 0;
static size_t cnt_open     = 0;

static struct cdev hcdev;
static struct class *devclass;

module_param_named( node_name, MOD_NAME, charp, 0 );

static int slt_open( struct inode *n, struct file *f )
{	
	++cnt_open;

	return 0;
}

static int slt_release( struct inode *n, struct file *f )
{	
	return 0;
}

static ssize_t slt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int ret; 
	if( *f_pos != 0 ) return 0;

	printk( KERN_INFO "%s : buf = %p count = %lu *f_pos = %llu\n", __FUNCTION__, buf, count, *f_pos );
	

	//ret = sprintf(buf, "%lu %lu\n", cnt_open, cnt_bytes_wr);
	ret = sprintf(buf, "%d\n", major);
	*f_pos = ret;
	printk( KERN_INFO "%s : cnt_open = %lu cnt_bytes_wr = %lu ret = %d",  __FUNCTION__, cnt_open, cnt_bytes_wr, ret );
	return ret;

}

static ssize_t slt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	printk( KERN_INFO "%s : buf = %p count = %lu *f_pos = %llu\n", __FUNCTION__, buf, count, *f_pos );

	cnt_bytes_wr += count;
	printk( KERN_INFO "%s : cnt_open = %lu cnt_bytes_wr = %lu",    __FUNCTION__, cnt_open, cnt_bytes_wr );

	return count;
}

static const struct file_operations slt_fops = {
	.owner   = THIS_MODULE,
	.open    = slt_open,
	.release = slt_release,
	.read    = slt_read,
	.write   = slt_write,
};

int slt_init(void)
{
	int ret;
	dev_t dev;

	//dev = MKDEV( major, minor );
	ret = alloc_chrdev_region( &dev, minor, 1, MOD_NAME );
	if( ret < 0 ) {
		printk( KERN_ERR "Can not register char device region\n" );
		return ret;
	}
	major = MAJOR( dev ); 

	/*ret = register_chrdev_region( dev, 1, MOD_NAME );
	if( ret < 0 ) {
		printk( KERN_ERR "Can not register char device region\n" );
		return ret;
	}*/

	cdev_init( &hcdev, &slt_fops);
	hcdev.owner = THIS_MODULE;
	//hcdev.ops = &slt_fops; // обязательно! - cdev_init() недостаточно?
	ret = cdev_add(&hcdev, dev, 1);
	if( ret < 0 ) {
		unregister_chrdev_region( MKDEV( major, minor ), 1 );
		printk( KERN_ERR "Can not add char device\n" );
		return ret;
	}

	devclass = class_create( THIS_MODULE, "dyn_class" );
	device_create( devclass, NULL, dev, "%s", MOD_NAME );

	printk( KERN_INFO "=========== module installed %d:%d ==============\n",
	MAJOR( dev ), MINOR( dev ) );
	
	return ret;
}

void slt_exit(void)
{
	device_destroy( devclass, MKDEV( major, minor ) );
	class_destroy( devclass );
	
	cdev_del( &hcdev );
	unregister_chrdev_region( MKDEV( major, minor ), 1 );
	printk( KERN_INFO "=============== module removed ==================\n" );

	return;
}


module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");