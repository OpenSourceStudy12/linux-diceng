KERNELDIR  ?=/mini2440/linux-2.6.32/include

all:beep_test
beep_test: beep_test.c
			arm-linux-gcc -I$(KERNELDIR) -o $@  $^
clean:
			rm -f main *~ beep_test
