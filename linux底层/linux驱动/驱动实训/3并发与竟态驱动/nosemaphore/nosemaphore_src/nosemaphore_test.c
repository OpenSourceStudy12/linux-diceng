#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int  argc, char  *argv[])
{
  int nosemaphore_fd;
  char wr_buf[60];
  int cnt=0;
  int input=0;
  if(argc > 1)
   { input = atoi(argv[1]); }
  else
   {
     printf("usage:nosem_test  number\n");
     exit(1);
   }
  nosemaphore_fd = open("/dev/pan_nosemaphore", O_RDWR | O_NONBLOCK);
  if(nosemaphore_fd==-1)
   {
     printf("cannot open file /dev/pan_nosemaphore\n");
     exit(1);
   }
  memset(wr_buf, input,  sizeof(wr_buf));

/*print  out  100 line  number, there are 60 one every line*/
  while(1)
   {
     write(nosemaphore_fd,  wr_buf,  sizeof(wr_buf));
     cnt++;
     if(cnt > 100)
      break;
     usleep(8);
   }

  close(nosemaphore_fd);
  return 0;
}
