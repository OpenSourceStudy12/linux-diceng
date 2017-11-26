/*******************************************************
  NAME: timekey_test.c

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
  int i, key_value[6];
  int ret, timekey_fd; 
  fd_set  rfds; 

//打开键盘设备文件
   timekey_fd = open("/dev/pan_time_key", O_RDWR | O_NONBLOCK);  
    if(timekey_fd<0) 
       {    
           printf("cannot open /dev/pan_time_key\n"); 
           return -1;  
       }
    while(1)
     {
          FD_ZERO(&rfds);
          FD_SET(timekey_fd, &rfds);

          ret = select(timekey_fd+1, &rfds, NULL, NULL,  NULL);
          if(ret==0)
            printf("timeout.\n");
          //数据是否可获得？
          if(FD_ISSET(timekey_fd, &rfds))
           {
              ret = read(timekey_fd, key_value, sizeof(key_value));
               if(ret == sizeof(key_value))
               for(i=0;i<6;i++)
                {
                   if(key_value[i])
                   printf("key%d is pressed", i+1);
                }         
           }           

     }

        close(timekey_fd);
  return  0;
}




