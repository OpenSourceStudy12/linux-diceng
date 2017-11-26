KERNELDIR  ?=/mini2440/linux-2.6.32/include

all:globalmem_test
globalmem_test: globalmem_test.c
	arm-linux-gcc -I$(KERNELDIR) -o $@  $^
clean:
	rm -f main *~ globalmem_test
