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
#include <linux/poll.h>

#define  time_key_MAJOR  237   /*预设的time_key的主设备号*/
#define  MAX_KEYS        6     /*6个按键*/

static int  time_key_major = time_key_MAJOR;


MODULE_LICENSE("Dual BSD/GPL");

static int  ev_press=0;
static DECLARE_WAIT_QUEUE_HEAD(key_waitq);

static int key_values[6] = {0};

struct  key_pin
{
   int pin;
   int pin_setting;
};

static struct key_pin  key_time[]={
  { S3C2410_GPG(0),  S3C2410_GPIO_INPUT },
  { S3C2410_GPG(3),  S3C2410_GPIO_INPUT },
  { S3C2410_GPG(5),  S3C2410_GPIO_INPUT },
  { S3C2410_GPG(6),  S3C2410_GPIO_INPUT },
  { S3C2410_GPG(7),  S3C2410_GPIO_INPUT },
  { S3C2410_GPG(11), S3C2410_GPIO_INPUT },  
}; 

struct  time_key_dev
  {
     /*time_key设备结构体*/
     struct  cdev  time_key;
      /*定义内核定时器结构体*/
     struct  timer_list  key_time_key;
   }; 

 struct  time_key_dev   *time_key_devp ; 

/*定时器处理函数*/
static  int key_time_key_handle()
{
   int  down;
   static  unsigned  int  pressed[MAX_KEYS] = { 0 };
   int i;
   for(i=0;i<MAX_KEYS;i++)
    {
       down = s3c2410_gpio_getpin(key_time[i].pin);
       if(!down)  //KEY  pressed
        {
          pressed[i]++;
          if(pressed[i]>2) pressed[i] = 3;
        }
       if(pressed[i] == 3)
        {
          printk("key%d is pressed\n", i+1);
          key_values[i] = 1;
          ev_press = 1;
          wake_up_interruptible(&key_waitq);
        }
       if(down && pressed[i])
        {
          pressed[i] = 0;
          key_values[i] = 0;
        }
    }
   
   time_key_devp->key_time_key.expires = jiffies + HZ/10;
   add_timer(&time_key_devp->key_time_key);
   return 0;
} 

/*文件打开函数*/
int time_key_open(struct inode *inode, struct file *filp)
{
   int i;
   for(i=0;i<MAX_KEYS;i++)
   {
     s3c2410_gpio_cfgpin(key_time[i].pin, key_time[i].pin_setting);
   }
   /*初始化内核定时器结构体*/
  init_timer(&time_key_devp->key_time_key);
  time_key_devp->key_time_key.function = &key_time_key_handle;
  time_key_devp->key_time_key.expires = jiffies + HZ/10;
  /*添加内核定时器*/
     add_timer(&time_key_devp->key_time_key);      

  return 0;
}
/*文件释放函数*/
int time_key_release(struct inode *inode, struct file *filp)
{
   del_timer(&time_key_devp->key_time_key);
  return 0;
}

/* ioctl设备控制函数 */
static int time_key_ioctl(struct inode *inode, struct file *filp, unsigned  int cmd, unsigned long arg)
{
  return 0;
}

/*读函数*/
static ssize_t time_key_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
         ev_press = 0; 
            if(copy_to_user(buf, key_values, sizeof(key_values)))
              {
               return -EFAULT;
              }

  return sizeof(unsigned int);
}

static unsigned  int time_key_poll(struct file  *filp, poll_table  *wait)
{
    unsigned  int mask = 0;
    poll_wait(filp, &key_waitq, wait);
   if(ev_press)
     {
        mask |= POLLIN | POLLRDNORM;
     }
   return  mask;
}

/*写函数*/
static ssize_t time_key_write(struct file *filp, const char __user *buf, size_t size, loff_t *offt)
{
    
    return  0;
}

/*文件操作结构体*/
static struct file_operations time_key_fops =
{
  .owner = THIS_MODULE,
  .read = time_key_read,
  .write = time_key_write,
  .ioctl = time_key_ioctl,
  .open = time_key_open,
  .release = time_key_release,
  .poll = time_key_poll,  
};

/*初始化并注册cdev*/
static void time_key_setup_cdev(struct time_key_dev *dev,  int index)
{
  int err, devno = MKDEV(time_key_major, index);

  cdev_init(&dev->time_key, &time_key_fops);
  dev->time_key.owner = THIS_MODULE;
  dev->time_key.ops = &time_key_fops;
  err = cdev_add(&dev->time_key, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int time_key_init(void)
{
  int result, ret;
  dev_t devno = MKDEV(time_key_major, 0);

  /* 申请设备号*/
  if (time_key_major)
    result = register_chrdev_region(devno, 1, "pan_time_key");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_time_key");
    time_key_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"time_key: unable to get major %d\n", time_key_major);
    return result;
   }
  
/*动态申请结构体内存*/
   time_key_devp = kmalloc(sizeof(struct time_key_dev), GFP_KERNEL);
     if(!time_key_devp)
        {
          ret = -ENOMEM;
          goto  fail_malloc;
        }
  memset(time_key_devp, 0, sizeof(struct time_key_dev));
  time_key_setup_cdev(time_key_devp, 0);

  printk("pan_time_key installed, with major %d\n", time_key_major);
   return  0;

  fail_malloc: unregister_chrdev_region(devno, 1);
}

/*模块卸载函数*/
void time_key_exit(void)
{
  cdev_del(&time_key_devp->time_key);   /*注销cdev*/
  kfree(time_key_devp);
  unregister_chrdev_region(MKDEV(time_key_major, 0), 1); /*释放设备号*/
  printk("time_key device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

module_init(time_key_init);
module_exit(time_key_exit);
