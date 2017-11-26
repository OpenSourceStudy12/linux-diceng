#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

int main()
{
  int fd;
  int counter = 0;
  int old_counter = 0;
  
  fd = open("/dev/pan_time",O_RDONLY);
  if(fd != -1)
    {
      while(1)
         {  
            read(fd,  &counter, sizeof(unsigned int));
            if(counter != old_counter)
              {
                printf("seconds  after open /dev/second :%d\n", counter);
                old_counter =  counter ;
              }

         }  
    }
  else
    {
    printf("Device open failure\n");       
    } 

  return  0;   
}

