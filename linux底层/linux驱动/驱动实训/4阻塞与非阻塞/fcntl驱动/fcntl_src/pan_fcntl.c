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

#define FCNTL_SIZE	0x1000	/*全局内存最大4K字节*/
#define MEM_CLEAR 0x1  /*清0全局内存*/
#define FCNTL_MAJOR 224    /*预设的fcntl的主设备号*/

     MODULE_LICENSE("Dual BSD/GPL");


static int fcntl_major = FCNTL_MAJOR;
/*fcntl设备结构体*/
struct fcntl_dev                                     
{                                                        
  	   struct cdev cdev; /*cdev结构体*/                       
  		unsigned char mem[FCNTL_SIZE]; /*全局内存*/ 
   struct  fasync_struct  *async_queue;/*异步结构体*/      
};

struct fcntl_dev *fcntl_devp; /*设备结构体指针*/

static  int  fcntl_fasync(int  fd, struct file  *filp, int  mode)
{
   struct  fcntl_dev  *dev = filp->private_data;
   return  fasync_helper(fd, filp, mode, &dev->async_queue);
}

/* ioctl设备控制函数 */
static int fcntl_ioctl(struct inode *inodep, struct file *filp, unsigned
  				int cmd, unsigned long arg)
{
  				struct fcntl_dev *dev = filp->private_data;/*获得设备结构体指针*/

  				switch (cmd)
  				{
    				case MEM_CLEAR:
      				memset(dev->mem, 0, FCNTL_SIZE);      
      				printk(KERN_INFO "fcntl is set to zero\n");
      				break;

    				default:
      				return  - EINVAL;
  				}
  				return 0;
}

/*读函数*/
static ssize_t fcntl_read(struct file *filp, char __user *buf, size_t size,loff_t *ppos)
{
  				unsigned long p =  *ppos;
  				unsigned int count = size;
  				int ret = 0;
  				struct fcntl_dev *dev = filp->private_data; /*获得设备结构体指针*/

  				/*分析和获取有效的写长度*/
  				if (p >= FCNTL_SIZE)
    				return count ?  - ENXIO: 0;
  				if (count > FCNTL_SIZE - p)
    				count = FCNTL_SIZE - p;

  				/*内核空间->用户空间*/
  				if (copy_to_user(buf, (void*)(dev->mem + p), count))
  				{
    				ret =  - EFAULT;
  				}
  				else
  				{
    				*ppos += count;
    				ret = count;
    
    				printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  				}

  				return ret;
}

/*写函数*/
static ssize_t fcntl_write(struct file *filp, const char __user *buf,  size_t size, loff_t *ppos)
{
  				unsigned long p =  *ppos;
  				unsigned int count = size;
  				int ret = 0;
  				struct fcntl_dev *dev = filp->private_data; /*获得设备结构体指针*/
  
  				/*分析和获取有效的写长度*/
  				if (p >= FCNTL_SIZE)
    			return count ?  - ENXIO: 0;
  				if (count > FCNTL_SIZE - p)
    			count = FCNTL_SIZE - p;
    
  				/*用户空间->内核空间*/
  				if (copy_from_user(dev->mem + p, buf, count))
    			ret =  - EFAULT;
  				else
  				{
    			*ppos += count;
    			ret = count;
    
    				printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
  					}

           if(dev->async_queue)
                kill_fasync(&dev->async_queue, SIGIO, POLL_IN) ;        

  				return ret;
}

/* seek文件定位函数 */
static loff_t fcntl_llseek(struct file *filp, loff_t offset, int orig)
{
  				loff_t ret = 0;
  				switch (orig)
  				{
    				case 0:   /*相对文件开始位置偏移*/
      				if (offset < 0)
      				{
        				ret =  - EINVAL;
        				break;
      				}
      				if ((unsigned int)offset > FCNTL_SIZE)
      				{
        				ret =  - EINVAL;
        				break;
      				}
      				filp->f_pos = (unsigned int)offset;
      				ret = filp->f_pos;
      				break;
    				case 1:   /*相对文件当前位置偏移*/
      				if ((filp->f_pos + offset) > FCNTL_SIZE)
      				{
        				ret =  - EINVAL;
        				break;
      				}
      				if ((filp->f_pos + offset) < 0)
      				{
        				ret =  - EINVAL;
        				break;
      				}
      				filp->f_pos += offset;
      				ret = filp->f_pos;
      				break;
    				default:
      				ret =  - EINVAL;
      				break;
  				}
  					return ret;
}

/*文件打开函数*/
int fcntl_open(struct inode *inode, struct file *filp)
{
  				/*将设备结构体指针赋值给文件私有数据指针*/
  				filp->private_data = fcntl_devp;
  				return 0;
}
/*文件释放函数*/
int fcntl_release(struct inode *inode, struct file *filp)
{
          struct  fcntl_dev  *dev = filp->private_data;

             //将文件从异步通知列表中删除
            fcntl_fasync(-1, filp, 0);
  				return 0;
}

/*文件操作结构体*/
static const struct file_operations fcntl_fops =
{
 				 .owner = THIS_MODULE,
 				 .llseek = fcntl_llseek,
  				 .read = fcntl_read,
 				 .write = fcntl_write,
                .ioctl = fcntl_ioctl,
                .fasync = fcntl_fasync,
 				 .open = fcntl_open,
 				 .release = fcntl_release,
};

/*初始化并注册cdev*/
static void fcntl_setup_cdev(struct fcntl_dev *dev, int   index)
{
  				int err, devno = MKDEV(fcntl_major, index);

  				cdev_init(&dev->cdev, &fcntl_fops);
  				dev->cdev.owner = THIS_MODULE;
  				err = cdev_add(&dev->cdev, devno, 1);
  				if (err)
    				printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*设备驱动模块加载函数*/
int fcntl_init(void)
{
  				int result;
  				dev_t devno = MKDEV(fcntl_major, 0);

  				/* 申请设备号*/
  				if (fcntl_major)
    				result = register_chrdev_region(devno, 1, "pan_fcntl");
  				else  /* 动态申请设备号 */
  				{
    				result = alloc_chrdev_region(&devno, 0, 1, "pan_fcntl");
    				fcntl_major = MAJOR(devno);
  				}  
  				if (result < 0)
    				return result;
    
  			/* 动态申请设备结构体的内存*/
  				fcntl_devp = kmalloc(sizeof(struct fcntl_dev), GFP_KERNEL);
  				if (!fcntl_devp)    /*申请失败*/
  				{
    				result =  - ENOMEM;
    				goto fail_malloc;
  				}
  				memset(fcntl_devp, 0, sizeof(struct fcntl_dev));
  
  				fcntl_setup_cdev(fcntl_devp, 0);
          printk("pan_fcntl device installed, with major %d\n", fcntl_major);
  				return 0;

  				fail_malloc: unregister_chrdev_region(devno, 1);
  				return result;
}

/*模块卸载函数*/
void fcntl_exit(void)
{
  				cdev_del(&fcntl_devp->cdev);   /*注销cdev*/
  				kfree(fcntl_devp);     /*释放设备结构体内存*/
  				unregister_chrdev_region(MKDEV(fcntl_major, 0), 1); /*释放设备号*/
}

MODULE_AUTHOR("xxxxxxxxxx");

module_param(fcntl_major, int, S_IRUGO);

module_init(fcntl_init);
module_exit(fcntl_exit);
