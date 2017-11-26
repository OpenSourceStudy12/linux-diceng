KERNELDIR  ?=/mini2440/linux-2.6.32/include

all:fcntl_test
fcntl_test: fcntl_test.c
	         arm-linux-gcc -I$(KERNELDIR) -o $@  $^
clean:
	         rm -f   *~ fcntl_test
