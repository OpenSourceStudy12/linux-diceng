/*******************************************************
  NAME: poll_test.c

*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

int main(void)
{  
  int num,poll_fd; 
  fd_set  rfds; 
  struct  timeval tv;

//打开键盘设备文件
   poll_fd = open("/dev/pan_poll", O_RDWR);  
    if(poll_fd<0) 
       {    
           printf("cannot open /dev/pan_poll\n"); 
            return -1;  
       }

          FD_ZERO(&rfds);
          FD_SET(poll_fd, &rfds);
         //设置超时时间为8s、
           tv.tv_sec = 8;
           tv.tv_usec = 0;

           select(poll_fd+1, &rfds, NULL, NULL,  &tv);
          
          //数据是否可获得？
          if(FD_ISSET(poll_fd, &rfds))
           {
              read(poll_fd, &num, sizeof(int));
              printf("the number is  %d\n", num);       
           }           
          else
            printf("No data within 8 seconds.\n");
        close(poll_fd);
  return  0;
}




