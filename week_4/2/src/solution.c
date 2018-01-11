/*
 * Разработать модуль ядра solution, который реализует интерфейс драйвера символьного устройства с обработчиком команд ioctl.
 * В качестве устройства необходимо использовать уже существующий node /dev/solution_node с major = 240. Драйвер работает в среде, 
 * где исключена необходимость синхронизации параллельных попыток доступа к устройству. Обработчики команд ioctl:

 * команда SUM_LENGTH, аргумент - указатель на массив char (длиной не более 20 элементов), действие - подсчитать длину строки, 
 * аккумулировать, вернуть текущую сумму.
 * команда SUM_CONTENT, аргумент - указатель на массив char (длиной не более 20 элементов), действие - сконвертировать в int, 
 * аккумулировать и вернуть текущую сумму.
 * Объявление команд:
 * 
 * #define IOC_MAGIC 'k'
 * 
 * #define SUM_LENGTH _IOWR(IOC_MAGIC, 1, char*)
 * #define SUM_CONTENT _IOWR(IOC_MAGIC, 2, char*)
 * 
 * Результаты работы драйвера должны быть доступны при чтении с символьного устройства /dev/solution_node в следующем формате
 * 
 * M N\n
 * где M - сумма длинн строк, присланных через команду 4444; N - сумма чисел, присланных через команду 5555.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


#define MOD_NAME "solution_node"
static const int major = 240;
static const int minor = 0;

#define IOC_MAGIC 'k'
#define SUM_LENGTH  _IOWR(IOC_MAGIC, 1, char*)
#define SUM_CONTENT _IOWR(IOC_MAGIC, 2, char*)

static size_t  sum_length_data  = 0;
static ssize_t sum_content_data = 0;

static struct cdev hcdev;

#define STR_BUF_SIZE 20
static char str_buf[STR_BUF_SIZE + 1];

static int slt_open( struct inode *n, struct file *f )
{	
	printk( KERN_INFO "kernel_mooc %s\n",  __FUNCTION__ );
	return 0;
}

static int slt_release( struct inode *n, struct file *f )
{	
	return 0;
}


static ssize_t slt_read(struct file *f, char __user *buf, size_t count, loff_t *f_pos)
{	
	printk( KERN_INFO "kernel_mooc %s : sum_length_data = %lu sum_content_data = %zd\n", __FUNCTION__, sum_length_data,  sum_content_data); 

	return sprintf( buf, "%lu %zd\n", sum_length_data, sum_content_data );
}

static long slt_ioctl( struct file *f, unsigned int cmd, unsigned long arg ) {
	
	int i;

	printk( KERN_INFO "kernel_mooc %s : cmd = %d _IOC_SIZE( cmd ) = %d\n", __FUNCTION__, cmd,  _IOC_SIZE( cmd ));

	if( ( _IOC_TYPE( cmd ) != IOC_MAGIC ) )   return -ENOTTY;
	if( _IOC_SIZE( cmd ) > sizeof(str_buf))   return -EINVAL;

	switch( cmd ) {
		case SUM_LENGTH:
			if( copy_from_user( str_buf, (void*)arg,  _IOC_SIZE( cmd ) ) ) return -EFAULT;
			str_buf[STR_BUF_SIZE] = 0;
			sum_length_data += strlen(str_buf);	

			printk( KERN_INFO "kernel_mooc %s : str_buf = %s\n", __FUNCTION__, str_buf);
			printk( KERN_INFO "kernel_mooc %s : strlen(str_buf) = %zd sum_length_data = %zd\n", __FUNCTION__, strlen(str_buf), sum_length_data);
			
			return sum_length_data;
		case SUM_CONTENT:
			if( copy_from_user( str_buf, (void*)arg,  _IOC_SIZE( cmd ) ) ) return -EFAULT;
			str_buf[STR_BUF_SIZE] = 0;
			sscanf(str_buf, "%d", &i);
			sum_content_data += i;

			printk( KERN_INFO "kernel_mooc %s : str_buf = %s\n", __FUNCTION__, str_buf);
			printk( KERN_INFO "kernel_mooc %s : i = %d sum_content_data = %zd\n", __FUNCTION__, i, sum_content_data);

			return sum_content_data;
		default:
			return -ENOTTY;
	}
	return 0;
}

static const struct file_operations slt_fops = {
	.owner   = THIS_MODULE,
	.open    = slt_open,
	.release = slt_release,
	.read    = slt_read,
	.unlocked_ioctl   = slt_ioctl
};

int slt_init(void)
{
	int ret;
	dev_t dev;

	dev = MKDEV( major, minor );
	ret = register_chrdev_region( dev, 1, MOD_NAME );
	if( ret < 0 ) {
		printk( KERN_ERR "kernel_mooc %s : Can not register char device region\n",  __FUNCTION__ );
		return ret;
	}

	cdev_init( &hcdev, &slt_fops);
	hcdev.owner = THIS_MODULE;
	//hcdev.ops = &slt_fops; // обязательно! - cdev_init() недостаточно?
	ret = cdev_add(&hcdev, dev, 1);
		if( ret < 0 ) {
		unregister_chrdev_region( MKDEV( major, minor ), 1 );
		printk( KERN_ERR "kernel_mooc %s : Can not add char device\n",  __FUNCTION__ );
		return ret;
	}

	printk( KERN_INFO "kernel_mooc %s : =========== module installed %d:%d ==============\n", __FUNCTION__ ,
	MAJOR( dev ), MINOR( dev ) );
	
	return ret;
}

void slt_exit(void)
{
	
	cdev_del( &hcdev );
	unregister_chrdev_region( MKDEV( major, minor ), 1 );
	printk( KERN_INFO "kernel_mooc %s : =============== module removed ==================\n", __FUNCTION__);

	return;
}


module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");