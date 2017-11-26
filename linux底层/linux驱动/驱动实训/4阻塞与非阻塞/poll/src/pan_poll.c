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
#include <linux/wait.h>

#include <linux/time.h>
#include <linux/poll.h>


#define  poll_MAJOR 239   /*预设的poll的主设备号*/


MODULE_LICENSE("Dual BSD/GPL");

static int  flag=0;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static  int  number=0;


static int poll_major = poll_MAJOR;
/*poll设备结构体*/
static struct  cdev  Poll;

/*poll start*/
void beep_start(void)
{
   s3c2410_gpio_pullup(S3C2410_GPB(0),  1);
   //set GPB0 as output
   s3c2410_gpio_cfgpin(S3C2410_GPB(0),  S3C2410_GPIO_OUTPUT);
   // set GPB0 as high
   s3c2410_gpio_setpin(S3C2410_GPB(0),  1);
}

/*poll stop*/
void beep_stop(void)
{
   //set GPB5-8 as output
   s3c2410_gpio_cfgpin(S3C2410_GPB(0),  S3C2410_GPIO_OUTPUT);
   // set GPB5-8 as low
   s3c2410_gpio_setpin(S3C2410_GPB(0),  0);
}

/*文件打开函数*/
int poll_open(struct inode *inode, struct file *filp)
{
  return 0;
}
/*文件释放函数*/
int poll_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* ioctl设备控制函数 */
static int poll_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  return 0;
}

/*读函数*/
static ssize_t poll_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
         flag = 0;
         beep_stop(); 
        if(copy_to_user(buf, &number, sizeof(int)))
          {
             return -EFAULT;
          } 
  return sizeof(int);
}

/*写函数*/
static ssize_t poll_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    int  ret;
    if(copy_from_user(&number, buf, sizeof(int)))
    {
       ret= -EFAULT;
    }
    flag=1;
    wake_up_interruptible(&wq);
    return  ret;
}

static unsigned  int  poll_poll(struct file  *filp, poll_table  *wait)
{
    unsigned  int mask = 0;
    poll_wait(filp, &wq, wait);
    beep_start();
   if(flag != 0)
     {
        mask |= POLLIN | POLLRDNORM;
        printk("poll retrun\n");
     }
   return  mask;
}
/*文件操作结构体*/
static struct file_operations poll_remap_fops =
{
  .owner = THIS_MODULE,
  .read = poll_read,
  .write = poll_write,
  .ioctl = poll_ioctl,
  .poll = poll_poll,
  .open = poll_open,
  .release = poll_release,
};

/*初始化并注册cdev*/
static void poll_setup_cdev(struct cdev *dev,  int minor,  struct  file_operations  *fops)
{
  int err, devno = MKDEV(poll_major, minor);

  cdev_init(dev, fops);
  dev->owner = THIS_MODULE;
  dev->ops = fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, minor);
}

/*设备驱动模块加载函数*/
int poll_init(void)
{
  int result;
  dev_t devno = MKDEV(poll_major, 0);

  /* 申请设备号*/
  if (poll_major)
    result = register_chrdev_region(devno, 1, "pan_poll");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_poll");
    poll_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"poll: unable to get major %d\n", poll_major);
    return result;
   }
  poll_setup_cdev(&Poll, 0,  &poll_remap_fops);
  printk("pan_poll installed, with major %d\n", poll_major);
  return   0;
}

/*模块卸载函数*/
void poll_exit(void)
{
  cdev_del(&Poll);   /*注销cdev*/
  unregister_chrdev_region(MKDEV(poll_major, 0), 1); /*释放设备号*/
  printk("poll device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

EXPORT_SYMBOL(poll_major);
module_init(poll_init);
module_exit(poll_exit);
