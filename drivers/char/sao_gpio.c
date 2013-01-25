#include<linux/module.h>
#include<linux/types.h>
#include<linux/fs.h>
#include<linux/mm.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/gpio.h>
#include<linux/io.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include<linux/serial.h>
MODULE_LICENSE("GPL");

struct cdev IO_cdev;
int IO_major = 243;
struct class *myclass;

static int IO_open(struct inode *inodep,struct file *filep)
{	
	gpio_free(S5PV210_GPH3(4));
	if(!gpio_request(S5PV210_GPH3(4),"IO_PORT_ON_Saoma"))
		printk(KERN_NOTICE"request %d OK\n",S5PV210_GPH3(4));
	else 
		return -1;
	gpio_direction_output(S5PV210_GPH3(4),1);
	//s3c_gpio_cfgpin(S5PV210_GPH3(4), S3C_GPIO_OUTPUT);	
	return 0;
}

static long IO_ioctl(struct file *filep,unsigned int cmd,unsigned long arg)
{
	if(cmd){
		gpio_set_value(S5PV210_GPH3(4),1);
	}
	else
	{
		gpio_set_value(S5PV210_GPH3(4),0);
	}
	return 0;
}
static int IO_release(struct inode *inodep,struct file *filep)
{
	gpio_free(S5PV210_GPH3(4));
	return 0;
}

static const struct file_operations IO_port_fops = 
{
	.owner = THIS_MODULE,
	.open = IO_open,
	.unlocked_ioctl = IO_ioctl,
	.release = IO_release,
};

static void setup_cdev(struct cdev *dev,int index)
{
	int devno = MKDEV(IO_major,index);
	cdev_init(dev,&IO_port_fops);
	dev->owner = THIS_MODULE;
	dev->ops = &IO_port_fops;
	cdev_add(dev,devno,1);
}

static int __init IO_init()
{
	dev_t devno = MKDEV(IO_major,0);
	int result;
	if(IO_major)
		result = register_chrdev_region(devno,1,"IO_ON_Saoma");
	else
	{
		result = alloc_chrdev_region(&devno,0,1,"IO_ON_Saoma");
		IO_major = MAJOR(devno);
	}
	memset(&IO_cdev,0x00,sizeof(IO_cdev));
	setup_cdev(&IO_cdev,0);
	myclass = class_create(THIS_MODULE,"IO_ON_Saoma_driver");
	device_create(myclass,NULL,MKDEV(IO_major,0),NULL,"IO_ON_Saoma");


	printk(KERN_NOTICE"init OK\n");
	return 0;
}

static void __exit IO_exit()
{
	cdev_del(&IO_cdev);
	device_destroy(myclass,MKDEV(IO_major,0));
	class_destroy(myclass);
	unregister_chrdev_region(MKDEV(IO_major,0),1);
	
	printk(KERN_NOTICE"exit  OK\n");
}
module_init(IO_init);
module_exit(IO_exit);

































