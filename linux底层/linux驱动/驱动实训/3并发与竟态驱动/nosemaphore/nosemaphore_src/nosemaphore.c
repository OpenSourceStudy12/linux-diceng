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


#define  NOSEMAPHORE_MAJOR 242    /*预设的NOSEMAPHORE的主设备号*/

#define  WRITE_SIZE    60
MODULE_LICENSE("Dual BSD/GPL");

unsigned char buff[WRITE_SIZE];

static int nosemaphore_major = NOSEMAPHORE_MAJOR;
/*beep设备结构体*/
static struct  cdev  NoSemaphore;

/*文件打开函数*/
int nosemaphore_open(struct inode *inode, struct file *filp)
{
  return 0;
}
/*文件释放函数*/
int nosemaphore_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/*读函数*/
static ssize_t nosemaphore_read(struct file *filp, char __user *buf, size_t size, loff_t *offt)
{
  return 0;
}

/*写函数*/
static ssize_t nosemaphore_write(struct file *filp, const char __user *buf,  size_t size, loff_t *offt)
{
  int  i;
  int  ret;
  unsigned  long p=*offt;
  unsigned  count=size;
 /*分析和获取有效的写长度*/
  if (p >= WRITE_SIZE)
    return count ?  - ENXIO: 0;
  if (count > WRITE_SIZE - p)
    count = WRITE_SIZE - p;
    
  /*用户空间->内核空间*/
  if (copy_from_user(buff, buf, count))
    ret =  - EFAULT;
  for(i=0; i<count; i++)
    {
      printk("%d", buff[i]);
    }
  return 0;
}

/*文件操作结构体*/
static struct file_operations nosemaphore_fops =
{
  .owner = THIS_MODULE,
  .read = nosemaphore_read,
  .write = nosemaphore_write,
  .open = nosemaphore_open,
  .release = nosemaphore_release,
};

/*初始化并注册cdev*/
static void nosemaphore_setup_cdev(struct cdev *dev,  int minor,  struct  file_operations  *fops)
{
  int err, devno = MKDEV(nosemaphore_major, minor);

  cdev_init(dev, fops);
  dev->owner = THIS_MODULE;
  dev->ops = fops;
  err = cdev_add(dev, devno, 1);
  if (err)
    printk(KERN_NOTICE "Error %d adding LED%d", err, minor);
}

/*设备驱动模块加载函数*/
int nosemaphore_init(void)
{
  int result;
  dev_t devno = MKDEV(nosemaphore_major, 0);

  /* 申请设备号*/
  if (nosemaphore_major)
    result = register_chrdev_region(devno, 1, "pan_nosemaphore");
  else  /* 动态申请设备号 */
   {
    result = alloc_chrdev_region(&devno, 0, 1, "pan_nosemaphore");
    nosemaphore_major = MAJOR(devno);
   }  
  if (result < 0)
   {
    printk(KERN_WARNING"nosemaphore: unable to get major %d\n", nosemaphore_major);
    return result;
   }
  nosemaphore_setup_cdev(&NoSemaphore, 0,  &nosemaphore_fops);
  printk("nosemaphore device installed, with major %d\n", nosemaphore_major);
  return   0;
}

/*模块卸载函数*/
void nosemaphore_exit(void)
{
  cdev_del(&NoSemaphore);   /*注销cdev*/
  unregister_chrdev_region(MKDEV(nosemaphore_major, 0), 1); /*释放设备号*/
  printk("nosemaphore device uninstalled\n");
}

MODULE_AUTHOR("xxxxxxxxx");

EXPORT_SYMBOL(nosemaphore_major);
module_init(nosemaphore_init);
module_exit(nosemaphore_exit);
