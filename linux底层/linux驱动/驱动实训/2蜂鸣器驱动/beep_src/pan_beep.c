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
#include <linux/ioctl.h>

#define  BEEP_MAJOR 243    /*预设的beep的主设备号*/

#define  BEEP_START_CMD    _IO('k', 1)
#define  BEEP_STOP_CMD     _IO('K', 0)

MODULE_LICENSE("Dual BSD/GPL");

static int beep_major = BEEP_MAJOR;
/*beep设备结构体*/
static struct  cdev  BeepDev;
/*beep start*/
void beep_start(void)
{
   // set GPB0 as high
   s3c2410_gpio_setpin(S3C2410_GPB(0),  1);
}

/*beep start*/
void beep_stop(void)
{
   // set GPB0 as low
   s3c2410_gpio_setpin(S3C2410_GPB(0),  0);
}

/*文件打开函数*/
int beep_open(struct inode *inode, struct file *filp)
{
   s3c2410_gpio_pullup(S3C2410_GPB(0),  1);
   //set GPB0 as output
   s3c2410_gpio_cfgpin(S3C2410_GPB(0),  S3C2410_GPIO_OUTPUT);
  return 0;
}
/*文件释放函数*/
int beep_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* ioctl设备控制函数 */
static int beep_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  switch (cmd)
  {
    case  BEEP_START_CMD:
          beep_start();
    	  break;
    case  BEEP_STOP_CMD:
          beep_stop();
    	  break;
    default:
      	  break;
  }
  return 0;
}

/*读函数*/
static ssize_t beep_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
  return 0;
}

/*写函数*/
static ssize_t beep_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *offt)
{
  return 0;
}

/*文件操作结构体*/
static struct file_operations beep_remap_fops =
{
  .owner = THIS_MODULE,
  .read = beep_read,
  .write = beep_write,
  .ioctl = beep_ioctl,
  .open = beep_open,
  .release = beep_release,
};

/*初始化并注册cdev*/
static void beep_setup_cdev(struct cdev *dev,  int minor,  struct  file_operations  *fops)
{
  int err, devno = MKDEV(beep_major, minor);

  cdev_init(dev, fops);
  dev->owner = THIS_MODULE;
  dev->ops = fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, minor);
}

/*设备驱动模块加载函数*/
int beep_init(void)
{
  int result;
  dev_t devno = MKDEV(beep_major, 0);

  /* 申请设备号*/
  if (beep_major)
    result = register_chrdev_region(devno, 1, "pan_beep");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_beep");
    beep_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"beep: unable to get major %d\n", beep_major);
    return result;
   }
  beep_setup_cdev(&BeepDev, 0,  &beep_remap_fops);
  printk("beep device installed, with major %d\n", beep_major);
  return   0;
}

/*模块卸载函数*/
void beep_exit(void)
{
  cdev_del(&BeepDev);   /*注销cdev*/
  unregister_chrdev_region(MKDEV(beep_major, 0), 1); /*释放设备号*/
  printk("beep device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

EXPORT_SYMBOL(beep_major);
module_init(beep_init);
module_exit(beep_exit);
