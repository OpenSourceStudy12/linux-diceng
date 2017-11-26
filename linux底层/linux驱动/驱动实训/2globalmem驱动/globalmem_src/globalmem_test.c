#include <stdio.h>
#include <fcntl.h>

#define   XXX_IOCTL_CMD   0
int main()
{
  int dev_fd;
  char read_buf[10];
  char write_buf[]="hello how are you";
  dev_fd = open("/dev/globalmem", O_RDWR | O_NONBLOCK);
  if(dev_fd==-1)
   {
     printf("cannot open file /dev/globlmem\n");
     exit(1);
   }
  ioctl(dev_fd, XXX_IOCTL_CMD, 0);
  lseek(dev_fd, 0, 0);
  write(dev_fd, write_buf,  20);
  lseek(dev_fd, 0, 0);
  read(dev_fd, read_buf, 20);
  printf("read_buf=%s\n", read_buf);

  close(dev_fd);
  return 0;
}
