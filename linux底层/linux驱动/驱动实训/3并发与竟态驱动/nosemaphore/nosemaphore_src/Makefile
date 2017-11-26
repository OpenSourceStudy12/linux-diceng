KERNELDIR  ?=/mini2440/linux-2.6.32/include

all:nosemaphore_test
nosemaphore_test: nosemaphore_test.c
	arm-linux-gcc -I$(KERNELDIR) -o $@  $^
clean:
	rm -f main *~ nosemaphore_test
