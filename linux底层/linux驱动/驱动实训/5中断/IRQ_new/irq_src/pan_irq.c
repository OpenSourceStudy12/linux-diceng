#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/wait.h>

#define  PAN_IRQ_MAJOR  236    /*预设的pan_irq的主设备号*/
#define  MAX_KEYS        6     /*6个按键*/

static int  pan_irq_major = PAN_IRQ_MAJOR;

MODULE_LICENSE("Dual BSD/GPL");

static int  ev_press=0;
static DECLARE_WAIT_QUEUE_HEAD(pan_irq_waitq);

static int pan_irq_values[6] = {0};

struct  pan_irq_pin
{
   int irq;
   int pin;
   int pin_setting;
   unsigned  long  flags;
   char   *name;
};

static struct pan_irq_pin   pan_button_irq[] =
{
	{IRQ_EINT8 , S3C2410_GPG(0) , S3C2410_GPG0_EINT8 , IRQF_TRIGGER_FALLING, "KEY0"},
	{IRQ_EINT11, S3C2410_GPG(3) , S3C2410_GPG3_EINT11 , IRQF_TRIGGER_FALLING, "KEY1"},
	{IRQ_EINT13, S3C2410_GPG(5) , S3C2410_GPG5_EINT13 , IRQF_TRIGGER_FALLING, "KEY2"},
	{IRQ_EINT14, S3C2410_GPG(6) , S3C2410_GPG6_EINT14 , IRQF_TRIGGER_FALLING, "KEY3"},
	{IRQ_EINT15, S3C2410_GPG(7) , S3C2410_GPG7_EINT15 , IRQF_TRIGGER_FALLING, "KEY4"},
	{IRQ_EINT19, S3C2410_GPG(11) , S3C2410_GPG11_EINT19 , IRQF_TRIGGER_FALLING, "KEY5"},
}; 

 /*pan_irq设备结构体*/
 struct  cdev  *pan_irq_cdev;
 

/*定时器处理函数*/
static  irqreturn_t pan_irq_interrupt(int irq, void *dev_id, struct pt_regs  *regs)
{
   	volatile int *pan_irq_values = (volatile int *)dev_id;
	*pan_irq_values = *pan_irq_values + 1;
	ev_press = 1; /* 表示中断发生了 */
	wake_up_interruptible(&pan_irq_waitq); /* 唤醒休眠的进程 */
	return IRQ_RETVAL(IRQ_HANDLED);
} 

/*文件打开函数*/
int pan_irq_open(struct inode *inode, struct file *filp)
{
	int i,err;
	for (i = 0; i < sizeof(pan_button_irq)/sizeof(pan_button_irq[0]); i++)
	{   // 注册中断处理函数
	     err = request_irq(pan_button_irq[i].irq, pan_irq_interrupt,pan_button_irq[i].flags, pan_button_irq[i].name, (void *)&pan_irq_values[i]);
	  if (err)
	      break;
	 }
	 if (err)
	   {
	      // 释放已经注册的中断
	      i--;
	      for (; i >= 0; i--)
	      {
		free_irq(pan_button_irq[i].irq, (void *)&pan_irq_values[i]);
	      }
	      return -EBUSY;
	    }
	return 0;
}
/*文件释放函数*/
int pan_irq_release(struct inode *inode, struct file *filp)
{
	int i;
	for (i = 0; i < sizeof(pan_button_irq)/sizeof(pan_button_irq[0]); i++)
	{
		// 释放已经注册的中断
	free_irq(pan_button_irq[i].irq, (void *)&pan_irq_values[i]);
	}
	return 0;
}

/* ioctl设备控制函数 */
static int pan_irq_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  return 0;
}

/*读函数*/
static ssize_t pan_irq_read(struct file *filp, char __user *buff, size_t count, loff_t *offt)
{
        unsigned long err;
	ev_press = 0;
	/* 将按键状态复制给用户，并清0 */
	err = copy_to_user(buff, (const void *)pan_irq_values, min(sizeof(pan_irq_values), count));
	memset((void *)pan_irq_values, 0, sizeof(pan_irq_values));
	return err ? -EFAULT : 0;
}

static unsigned  int pan_irq_poll(struct file  *filp, poll_table  *wait)
{
    unsigned  int mask = 0;
    poll_wait(filp, &pan_irq_waitq, wait);
   if(ev_press)
     {
        mask |= POLLIN | POLLRDNORM;
     }
   return  mask;
}

/*写函数*/
static ssize_t pan_irq_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    
    return  0;
}

/*文件操作结构体*/
static struct file_operations pan_irq_fops =
{
  .owner = THIS_MODULE,
  .read = pan_irq_read,
  .write = pan_irq_write,
  .ioctl = pan_irq_ioctl,
  .open = pan_irq_open,
  .release = pan_irq_release,
  .poll = pan_irq_poll,  
};

/*初始化并注册cdev*/
static void pan_irq_setup_cdev(struct cdev *dev,  int index)
{
  int err, devno = MKDEV(pan_irq_major, index);

  cdev_init(dev, &pan_irq_fops);
  dev->owner = THIS_MODULE;
  dev->ops = &pan_irq_fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int pan_irq_init(void)
{
  int result, ret;
  dev_t devno = MKDEV(pan_irq_major, 0);

  /* 申请设备号*/
  if (pan_irq_major)
    result = register_chrdev_region(devno, 1, "pan_irq");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_irq");
    pan_irq_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"pan_irq: unable to get major %d\n", pan_irq_major);
    return result;
   }
  
/*动态申请结构体内存*/
   pan_irq_cdev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
     if(!pan_irq_cdev)
        {
          ret = -ENOMEM;
          goto  fail_malloc;
        }
  memset(pan_irq_cdev, 0, sizeof(struct cdev));
  pan_irq_setup_cdev(pan_irq_cdev, 0);

  printk("pan_irq installed, with major %d\n", pan_irq_major);
   return  0;

  fail_malloc: unregister_chrdev_region(devno, 1);
   return result;
}

/*模块卸载函数*/
void pan_irq_exit(void)
{
  cdev_del(pan_irq_cdev);   /*注销cdev*/
  kfree(pan_irq_cdev);
  unregister_chrdev_region(MKDEV(pan_irq_major, 0), 1); /*释放设备号*/
  printk("pan_irq device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

module_init(pan_irq_init);
module_exit(pan_irq_exit);
