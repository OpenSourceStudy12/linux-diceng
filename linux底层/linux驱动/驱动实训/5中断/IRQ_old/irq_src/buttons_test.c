/*******************************************************
  NAME: buttons_test.c

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
  int i;
  int ret;  
  int fd; 
  int press_cnt[4] = {0, 0, 0, 0};
  int sum[4] = {0, 0, 0, 0};


//打开键盘设备文件
   fd = open("/dev/pan-buttons", 0);  
    if(fd<0) 
       {    
           printf("cannot open /dev/pan-buttons\n"); 
            return -1;  
       }
     for(;;)
       {   
          int ret;
    
     /*开始读键盘驱动发出的数据，注意key_value和键盘驱动中定义为一致的类型*/
         ret = read(fd, press_cnt, sizeof(press_cnt));
           if(ret<0)
            {
                printf("read err!\n");
                  continue;
            }
           else 
          {
        /*打印键值*/
        for(i=0; i<4; i++)
         { sum[i] = sum[i] + press_cnt[i];
          if(press_cnt[i])
          printf("K%d has been pressed %d times!   sum = %d times!\n", i+1, press_cnt[i], sum[i]);
         }
       }
   }  
   close(fd); 
   return 0;
}
