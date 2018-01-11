/*
 * Разработать модуль ядра solution, который реализует файловый интерфейс драйвера символьного устройства. 
 * В качестве устройства необходимо использовать уже существующий node /dev/solution_node с major = 240.
 * Драйвер должен выделять отдельный участок памяти для каждого нового сеанса (под сеансом понимается период между открытием и закрытием файла устройства)
 * и поддерживать операции записи, чтения и изменение указателя текущей позиции (llseek). При этом драйвер должен обеспечивать корректную работу 
 * (в ходе каждого сеанса пользователь получает/пишет данные в отдельный, выделенный для данного сеанса буфер) при параллельном выполнении операций чтения, 
 * записи, открытия и закрытия файла устройства.
 * 
 * В ходе операции записи драйвер должен реализовывать запись строк длиной до 255 символов в текущую позицию указателя.
 * 
 * При чтении с символьного устройства /dev/solution_node драйвер должен возвращать данные в следующем формате:
 * 
 * S
 * 
 * где S - при первом обращении является порядковым номером сеанса (начиная с нуля), а при последующих - строкой,
 * находящейся на данный момент по указателю позиции для данного сеанса
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

static ssize_t cnt_open    = -1;

static struct cdev hcdev;
static const int BUF_SIZE  = 255;

static int slt_open( struct inode *n, struct file *f )
{	
	
	++cnt_open;
	f->private_data = kcalloc( BUF_SIZE  + 1 , 1, GFP_KERNEL );
	if( NULL == f->private_data ) return -ENOMEM;
	sprintf(f->private_data, "%lu", cnt_open);
	printk( KERN_INFO "kernel_mooc %s : cnt_open = %lu\n", __FUNCTION__,  cnt_open);
	return 0;
}

static int slt_release( struct inode *n, struct file *f )
{	
	kfree( f->private_data );
	return 0;
}


loff_t slt_llseek(struct file *f, loff_t offset, int origin)
{

	loff_t newpos;
	switch(origin) {
	case 0: /* SEEK_SET */
		newpos = offset;
		break;
	case 1: /* SEEK_CUR */
		newpos = f->f_pos + offset;
		break;
	case 2: /* SEEK_END */
		//newpos = BUF_SIZE + offset;
		 newpos = strlen(f->private_data ) + offset;
		break;
	default: /* can't happen */
		return -EINVAL;
	}
	newpos = newpos < BUF_SIZE ? newpos : BUF_SIZE;
	newpos = newpos >= 0 ? newpos : 0;
	f->f_pos = newpos;
	printk( KERN_INFO "kernel_mooc %s : newpos = %llu\n", __FUNCTION__, newpos);
	return newpos;

}

static ssize_t slt_read(struct file *f, char __user *buf, size_t count, loff_t *f_pos)
{
	
	size_t cur_size;

	cur_size = (*f_pos + count) < BUF_SIZE ? count : strlen(f->private_data + *f_pos);
	//cur_size = strlen(f->private_data + *f_pos);
	printk( KERN_INFO "kernel_mooc %s : count = %lu cur_size = %lu *f_pos = %llu\n", __FUNCTION__, count, cur_size, *f_pos);
	if(cur_size == 0) return 0;

	if(copy_to_user( (void*)buf, (char*)f->private_data + *f_pos,  cur_size ) == 0) {
		*f_pos += cur_size;
		return cur_size;
	}
	else return -EINVAL;

}

static ssize_t slt_write(struct file *f, const char __user *buf, size_t count, loff_t *f_pos)
{
	if((*f_pos + count) > BUF_SIZE) return 0;
	printk( KERN_INFO "kernel_mooc %s : buf = %p count = %lu *f_pos = %llu\n", __FUNCTION__, buf, count, *f_pos );

	if(copy_from_user( (char*)f->private_data + *f_pos, (void*)buf,  count) == 0) {
		*f_pos += count;
		return count;
	}
	else return -EINVAL;
}

static const struct file_operations slt_fops = {
	.owner   = THIS_MODULE,
	.llseek  = slt_llseek,
	.open    = slt_open,
	.release = slt_release,
	.read    = slt_read,
	.write   = slt_write,
};

int slt_init(void)
{
	int ret;
	dev_t dev;

	dev = MKDEV( major, minor );
	ret = register_chrdev_region( dev, 1, MOD_NAME );
	if( ret < 0 ) {
		printk( KERN_ERR "Can not register char device region\n" );
		return ret;
	}

	cdev_init( &hcdev, &slt_fops);
	hcdev.owner = THIS_MODULE;
	//hcdev.ops = &slt_fops; // обязательно! - cdev_init() недостаточно?
	ret = cdev_add(&hcdev, dev, 1);
		if( ret < 0 ) {
		unregister_chrdev_region( MKDEV( major, minor ), 1 );
		printk( KERN_ERR "Can not add char device\n" );
		return ret;
	}

	printk( KERN_INFO "=========== module installed %d:%d ==============\n",
	MAJOR( dev ), MINOR( dev ) );
	
	return ret;
}

void slt_exit(void)
{
	
	cdev_del( &hcdev );
	unregister_chrdev_region( MKDEV( major, minor ), 1 );
	printk( KERN_INFO "=============== module removed ==================\n" );

	return;
}


module_init(slt_init);
module_exit(slt_exit);
MODULE_LICENSE("GPL");