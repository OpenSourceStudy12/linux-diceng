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

#define  PAN_TASKLET_MAJOR  235    /*预设的pan_tasklet的主设备号*/
#define  MAX_KEYS        6     /*6个按键*/

static int  pan_tasklet_major = PAN_TASKLET_MAJOR;

MODULE_LICENSE("Dual BSD/GPL");

static int  ev_press=0;
static DECLARE_WAIT_QUEUE_HEAD(pan_tasklet_waitq);

static int pan_tasklet_values[6] = {0};

static struct  tasklet_struct  pan_tasklet;

struct  pan_tasklet_pin
{
   int  irq;
   int pin;
   int pin_setting;
   unsigned  long  flags;
   char   *name;
};

static struct pan_tasklet_pin   pan_button_tasklet[] =
{
	{IRQ_EINT8 , S3C2410_GPG(0) , S3C2410_GPG0_EINT8 , IRQF_TRIGGER_FALLING, "KEY0"},
	{IRQ_EINT11, S3C2410_GPG(3) , S3C2410_GPG3_EINT11 , IRQF_TRIGGER_FALLING, "KEY1"},
	{IRQ_EINT13, S3C2410_GPG(5) , S3C2410_GPG5_EINT13 , IRQF_TRIGGER_FALLING, "KEY2"},
	{IRQ_EINT14, S3C2410_GPG(6) , S3C2410_GPG6_EINT14 , IRQF_TRIGGER_FALLING, "KEY3"},
	{IRQ_EINT15, S3C2410_GPG(7) , S3C2410_GPG7_EINT15 , IRQF_TRIGGER_FALLING, "KEY4"},
	{IRQ_EINT19, S3C2410_GPG(11) , S3C2410_GPG11_EINT19 , IRQF_TRIGGER_FALLING, "KEY5"},
}; 

 /*pan_tasklet设备结构体*/
 struct  cdev  *pan_tasklet_cdev;
 
/*中断处理底半部*/
void pan_do_tasklet(unsigned long m)
{     
        printk("pan_tasklet_interrupt\n"); 	
	ev_press = 1; /* 表示中断发生了 */
	wake_up_interruptible(&pan_tasklet_waitq); /* 唤醒休眠的进程 */
	
} 

/*中断处理顶半部*/
static  irqreturn_t  pan_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
   volatile int *pan_tasklet_values =(volatile int *)dev_id;
   *pan_tasklet_values = *pan_tasklet_values + 1;
        printk("pan_interrupt\n");
   tasklet_schedule(&pan_tasklet);
   return IRQ_RETVAL(IRQ_HANDLED);
}

/*文件打开函数*/
int pan_tasklet_open(struct inode *inode, struct file *filp)
{
	int i,err;

       tasklet_init(&pan_tasklet, pan_do_tasklet, 0);

	for (i = 0; i < sizeof(pan_button_tasklet)/sizeof(pan_button_tasklet[0]); i++)
	{   // 注册中断处理函数
	     err = request_irq(pan_button_tasklet[i].irq, pan_interrupt,pan_button_tasklet[i].flags, pan_button_tasklet[i].name, (void *)&pan_tasklet_values[i]);
	  if (err)
	      break;
	 }
	 if (err)
	   {
	      // 释放已经注册的中断
	      i--;
	      for (; i >= 0; i--)
	      {
		free_irq(pan_button_tasklet[i].irq, (void *)&pan_tasklet_values[i]);
	      }
	      return -EBUSY;
	    }
	return 0;
}
/*文件释放函数*/
int pan_tasklet_release(struct inode *inode, struct file *filp)
{
	int i;
        tasklet_kill(&pan_tasklet);
	for (i = 0; i < sizeof(pan_button_tasklet)/sizeof(pan_button_tasklet[0]); i++)
	{
		// 释放已经注册的中断
	free_irq(pan_button_tasklet[i].irq, (void *)&pan_tasklet_values[i]);
	}
	return 0;
}

/* ioctl设备控制函数 */
static int pan_tasklet_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  return 0;
}

/*读函数*/
static ssize_t pan_tasklet_read(struct file *filp, char __user *buff, size_t count, loff_t *offt)
{
        unsigned long err;
	ev_press = 0;
	/* 将按键状态复制给用户，并清0 */
	err = copy_to_user(buff, (const void *)pan_tasklet_values, min(sizeof(pan_tasklet_values), count));
	memset((void *)pan_tasklet_values, 0, sizeof(pan_tasklet_values));
	return err ? -EFAULT : 0;
}

static unsigned  int pan_tasklet_poll(struct file  *filp, poll_table  *wait)
{
    unsigned  int mask = 0;
    poll_wait(filp, &pan_tasklet_waitq, wait);
   if(ev_press)
     {
        mask |= POLLIN | POLLRDNORM;
     }
   return  mask;
}

/*写函数*/
static ssize_t pan_tasklet_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    
    return  0;
}

/*文件操作结构体*/
static struct file_operations pan_tasklet_fops =
{
  .owner = THIS_MODULE,
  .read = pan_tasklet_read,
  .write = pan_tasklet_write,
  .ioctl = pan_tasklet_ioctl,
  .open = pan_tasklet_open,
  .release = pan_tasklet_release,
  .poll = pan_tasklet_poll,  
};

/*初始化并注册cdev*/
static void pan_tasklet_setup_cdev(struct cdev *dev,  int index)
{
  int err, devno = MKDEV(pan_tasklet_major, index);

  cdev_init(dev, &pan_tasklet_fops);
  dev->owner = THIS_MODULE;
  dev->ops = &pan_tasklet_fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int pan_tasklet_init(void)
{
  int result, ret;
  dev_t devno = MKDEV(pan_tasklet_major, 0);

  /* 申请设备号*/
  if (pan_tasklet_major)
    result = register_chrdev_region(devno, 1, "pan_tasklet");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_tasklet");
    pan_tasklet_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"pan_tasklet: unable to get major %d\n", pan_tasklet_major);
    return result;
   }
  
/*动态申请结构体内存*/
   pan_tasklet_cdev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
     if(!pan_tasklet_cdev)
        {
          ret = -ENOMEM;
          goto  fail_malloc;
        }
  memset(pan_tasklet_cdev, 0, sizeof(struct cdev));
  pan_tasklet_setup_cdev(pan_tasklet_cdev, 0);

  printk("pan_tasklet installed, with major %d\n", pan_tasklet_major);
   return  0;

  fail_malloc: unregister_chrdev_region(devno, 1);
   return result;
}

/*模块卸载函数*/
void pan_tasklet_exit(void)
{
  cdev_del(pan_tasklet_cdev);   /*注销cdev*/
  kfree(pan_tasklet_cdev);
  unregister_chrdev_region(MKDEV(pan_tasklet_major, 0), 1); /*释放设备号*/
  printk("pan_tasklet device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

module_init(pan_tasklet_init);
module_exit(pan_tasklet_exit);
