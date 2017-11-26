#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
   
  int poll_fd;
  int write_buf;
  poll_fd = open("/dev/pan_poll", O_RDWR);
  if(poll_fd==-1)
   {
     printf("cannot open file /dev/pan_poll\n");
     exit(1);
   }
  else
    printf("please input number:\n");
    scanf("%d",&write_buf);
    write(poll_fd, &write_buf, sizeof(int));
  return 0;
}
