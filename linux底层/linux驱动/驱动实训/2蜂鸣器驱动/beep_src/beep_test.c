#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define  BEEP_START_CMD    _IO('k', 1)
#define  BEEP_STOP_CMD     _IO('K', 0)

int main()
{
  int beep_fd;
  beep_fd = open("/dev/pan_beep", O_RDONLY | O_NONBLOCK);
  if(beep_fd==-1)
   {
     printf("cannot open file /dev/beep\n");
     exit(1);
   }
  while(1)
   {
     ioctl(beep_fd, BEEP_START_CMD, 0);
     sleep(3);
     ioctl(beep_fd, BEEP_STOP_CMD, 0);
     sleep(3);
   }
  close(beep_fd);
  return 0;
}
