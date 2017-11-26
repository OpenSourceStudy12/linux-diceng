/*******************************************************
  NAME: poll_test.c

*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <linux/poll.h>
#include <string.h>



int main(void)
{  
  int num,poll_fd; 
  int ret; 
  static struct  pollfd  bpoll[1];  

//打开键盘设备文件
   poll_fd = open("/dev/pan_poll", O_RDWR | O_NONBLOCK);  
    if(poll_fd<0) 
       {    
           printf("cannot open /dev/pan_poll\n"); 
            return -1;  
       }


           memset(bpoll,  0, sizeof(bpoll));

           bpoll[0].fd = poll_fd; //存放打开文件的文件描述符
           bpoll[0].events = POLLIN ;//存放要等待的事件，即唤醒条件          

          ret = poll(bpoll, 1 ,  8000);
          
          //数据是否可获得？
          if(ret < 0)
            {
              printf("poll error!\n");
            }
          if(ret == 0)
            {
              printf("time is out!\n");
            }
          if(bpoll[0].revents & POLLERR)
            {
              printf("Device error!\n");
              exit(1);
            }
          if(bpoll[0].revents & POLLIN)
           {
              read(poll_fd, &num, sizeof(int));
              printf("the number is  %d\n", num);       
           }           

        close(poll_fd);
        return  0;
}




