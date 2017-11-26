/*******************************************************
  NAME: pan_tasklet_test.c

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
  int fd; 
  int press_cnt[6]={0,0,0,0,0,0};
  int sum[6] = {0,0,0,0,0,0};


//打开键盘设备文件
   fd = open("/dev/pan_tasklet", 0);  
    if(fd<0) 
       {    
           printf("cannot open /dev/pan_tasklet\n"); 
            return -1;  
       }
     for(;;)
       {   
          int ret;
          fd_set   rds;
          FD_ZERO(&rds);
          FD_SET(fd, &rds);
          
          ret = select(fd+1, &rds, NULL, NULL, NULL);
          if(ret<0)
              {
                perror("select error!");
                 exit(1);
               }
          if(ret==0)
              printf("select timeout!\n");
          if(FD_ISSET(fd, &rds)) 
          {
          /*开始读键盘驱动发出的数据，注意key_value和键盘驱动中定义为一致的类型*/
            ret = read(fd, press_cnt, sizeof(press_cnt));
          /*打印键值*/
            for(i=0; i<6; i++)
              { 
                sum[i] = sum[i] + press_cnt[i];
                if(press_cnt[i])
                printf("K%d has been pressed %d times!   sum = %d times!\n", i+1, press_cnt[i], sum[i]);
              }
           }
         }  
   close(fd); 
   return 0;
}
