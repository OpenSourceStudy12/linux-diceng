#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/gpio.h>

#define  KEY_MAJOR 220    /*预设的key的主设备号*/

#define  KEY_START_CMD    1
#define  KEY_STOP_CMD     0

MODULE_LICENSE("Dual BSD/GPL");

static int key_major = KEY_MAJOR;
/*key设备结构体*/
static struct  cdev  keyDev;

/*文件打开函数*/
int key_open(struct inode *inode, struct file *filp)
{
     s3c2410_gpio_cfgpin(S3C2410_GPG(0),  S3C2410_GPIO_INPUT);
  return 0;
}
/*文件释放函数*/
int key_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* ioctl设备控制函数 */
static int key_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  if( s3c2410_gpio_getpin(S3C2410_GPG(0)) ) 
    return   0;  
  else
    return 1;
}

/*读函数*/
static ssize_t key_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
  return 0;
}

/*写函数*/
static ssize_t key_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *offt)
{
  return 0;
}

/*文件操作结构体*/
static struct file_operations key_remap_fops =
{
  .owner = THIS_MODULE,
  .read = key_read,
  .write = key_write,
  .ioctl = key_ioctl,
  .open = key_open,
  .release = key_release,
};

/*初始化并注册cdev*/
static void key_setup_cdev(struct cdev *dev,  int minor,  struct  file_operations  *fops)
{
  int err, devno = MKDEV(key_major, minor);
  cdev_init(dev, fops);
  dev->owner = THIS_MODULE;
  dev->ops = fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, minor);
}

/*设备驱动模块加载函数*/
int  pan_key_init(void)
{
  int result;
  dev_t devno = MKDEV(key_major, 0);
  /* 申请设备号*/
  if(key_major)
    result = register_chrdev_region(devno, 1, "pan_key");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_key");
    key_major = MAJOR(devno);
   }  
  if(result < 0)
   {
    printk(KERN_WARNING"key: unable to get major %d\n", key_major);
    return result;
   }
  key_setup_cdev(&keyDev, 0,  &key_remap_fops);
  printk("key device installed, with major %d\n", key_major);
  return   0;
}

/*模块卸载函数*/
void pan_key_exit(void)
{
  cdev_del(&keyDev);   /*注销cdev*/
  unregister_chrdev_region(MKDEV(key_major, 0), 1); /*释放设备号*/
  printk("key device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");
module_init(pan_key_init);
module_exit(pan_key_exit);
