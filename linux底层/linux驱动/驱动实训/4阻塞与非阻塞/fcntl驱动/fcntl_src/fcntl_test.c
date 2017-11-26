#include  <sys/types.h>
#include  <sys/stat.h>
#include  <stdio.h>
#include  <fcntl.h>
#include  <signal.h>
#include  <unistd.h>

#define  MAX_LEN    30
 int dev_fd;
 char write_buf[MAX_LEN];

void  input_handler(int  num)
  {

       char  read_buf[MAX_LEN];
       int  len;
  		lseek(dev_fd, 0, 0);
       len = read(dev_fd, read_buf,  MAX_LEN);
       read_buf[len] = 0;
       printf("num =%d\n", num);
       printf("input available:%s\n", read_buf);
  }

int main()
{
         int  oflags;
         signal(SIGIO, input_handler);

       dev_fd = open("/dev/pan_fcntl", O_RDWR | O_NONBLOCK);
         fcntl(dev_fd, F_SETOWN, getpid());
         oflags = fcntl(dev_fd, F_GETFL);
         fcntl(dev_fd, F_SETFL, oflags | FASYNC);
 //        while(1)
//            {
                printf("please input string:\n ");
                scanf("%s", write_buf);
                ioctl(dev_fd, 0, 0);
  		          lseek(dev_fd, 0, 0);
  		          write(dev_fd, write_buf,  MAX_LEN);
  //          }
         close(dev_fd);
      return  0;
}






