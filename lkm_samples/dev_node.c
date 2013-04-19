/* 3.2.0 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

static dev_t 		Dev;
static struct cdev 	Cdev;
static struct class 	*Class;

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length,
			   loff_t * offset)
{
	int bytes_read = 0;

	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, 
			    loff_t * off)
{
	return 0;
}

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = device_read,
	.write   = device_write,
	.open    = device_open,
	.release = device_release
};

static int __init mod_init(void)
{
	if (alloc_chrdev_region(&Dev, 0, 1, "chrDevName") < 0)
		goto out;

	if ((Class = class_create(THIS_MODULE, "charDrv")) == NULL)
		goto region;

	if (device_create(Class, NULL, Dev, NULL, "myDevice") == NULL)
		goto class;

	cdev_init(&Cdev, &fops);
	if (cdev_add(&Cdev, Dev, 1) == -1)
		goto device;

 	return 0;

device:
	device_destroy(Class, Dev);
class:
	class_destroy(Class);
region:
	unregister_chrdev_region(Dev, 1);
out:
	return -1;
}

static void __exit mod_exit(void)
{
	cdev_del(&Cdev);
	device_destroy(Class, Dev);
	class_destroy(Class);
	unregister_chrdev_region(Dev, 1);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("n/a");
MODULE_DESCRIPTION("n/a");
MODULE_LICENSE("GPL");

