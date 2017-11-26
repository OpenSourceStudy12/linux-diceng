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

#include <linux/timer.h>

#define  time_MAJOR  238   /*预设的time的主设备号*/


MODULE_LICENSE("Dual BSD/GPL");

static int  time_major = time_MAJOR;

struct  time_dev
  {
     /*time设备结构体*/
     struct  cdev  time;
     /*一共经历了多少秒，atomic_t是内核提供的一种原子的整数类型*/
     atomic_t  counter;
      /*定义内核定时器结构体*/
     struct  timer_list  key_timer;
   }; 

 struct  time_dev   *time_devp ; 

/*定时器处理函数*/
static  void key_timer_handle(unsigned long  arg)
{
   mod_timer(&time_devp->key_timer,  jiffies + HZ);
   atomic_inc(&time_devp->counter);
   printk(KERN_NOTICE"current jiffies  is  %ld\n", jiffies);
} 

/*文件打开函数*/
int time_open(struct inode *inode, struct file *filp)
{
   /*初始化内核定时器结构体*/
  init_timer(&time_devp->key_timer);
  time_devp->key_timer.function = &key_timer_handle;
  time_devp->key_timer.expires = jiffies + HZ;
  /*添加内核定时器*/
     add_timer(&time_devp->key_timer);      
  /*计数清零*/
     atomic_set(&time_devp->counter,0); 

  return 0;
}
/*文件释放函数*/
int time_release(struct inode *inode, struct file *filp)
{
   del_timer(&time_devp->key_timer);
  return 0;
}

/* ioctl设备控制函数 */
static int time_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  return 0;
}

/*读函数*/
static ssize_t time_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
         int  counter;
         counter = atomic_read(&time_devp->counter);
        if(copy_to_user(buf, &counter, sizeof(unsigned int)))
          {
             return -EFAULT;
          } 
  return sizeof(unsigned int);
}

/*
static ssize_t time_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
         int  counter;
         counter = atomic_read(&time_devp->counter);
        if(put_user(counter, (int *)buf))
          {
             return -EFAULT;
          } 
  return sizeof(unsigned int);
}
*/

/*写函数*/
static ssize_t time_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    
    return  0;
}

/*文件操作结构体*/
static struct file_operations time_fops =
{
  .owner = THIS_MODULE,
  .read = time_read,
  .write = time_write,
  .ioctl = time_ioctl,
  .open = time_open,
  .release = time_release,
};

/*初始化并注册cdev*/
static void time_setup_cdev(struct time_dev *dev,  int index)
{
  int err, devno = MKDEV(time_major, index);

  cdev_init(&dev->time, &time_fops);
  dev->time.owner = THIS_MODULE;
  dev->time.ops = &time_fops;
  err = cdev_add(&dev->time, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int time_init(void)
{
  int result, ret;
  dev_t devno = MKDEV(time_major, 0);

  /* 申请设备号*/
  if (time_major)
    result = register_chrdev_region(devno, 1, "pan_time");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_time");
    time_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"time: unable to get major %d\n", time_major);
    return result;
   }
  
/*动态申请结构体内存*/
   time_devp = kmalloc(sizeof(struct time_dev), GFP_KERNEL);
     if(!time_devp)
        {
          ret = -ENOMEM;
          goto  fail_malloc;
        }
  memset(time_devp, 0, sizeof(struct time_dev));
  time_setup_cdev(time_devp, 0);

  printk("pan_time installed, with major %d\n", time_major);
   return  0;

  fail_malloc: unregister_chrdev_region(devno, 1);
}

/*模块卸载函数*/
void time_exit(void)
{
  cdev_del(&time_devp->time);   /*注销cdev*/
  kfree(time_devp);
  unregister_chrdev_region(MKDEV(time_major, 0), 1); /*释放设备号*/
  printk("time device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

module_init(time_init);
module_exit(time_exit);
