#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define  BEEP_START_CMD    _IO('k', 1)
#define  BEEP_STOP_CMD     _IO('K', 0)

int main()
{
  int beep_fd, key_fd;
  beep_fd = open("/dev/pan_beep", O_RDONLY | O_NONBLOCK);
  if(beep_fd==-1)
   {
     printf("cannot open file /dev/key\n");
     exit(1);
   }
  key_fd = open("/dev/pan_key", O_RDONLY | O_NONBLOCK);
  if(key_fd==-1)
   {
     printf("cannot open file /dev/key\n");
     exit(1);
   }
  while(1)
   {
       if(ioctl(key_fd, 0, 0))
     ioctl(beep_fd, BEEP_START_CMD, 0);
        else
     ioctl(beep_fd, BEEP_STOP_CMD, 0);
   }
  close(beep_fd);
  close(key_fd);
  return 0;
}
