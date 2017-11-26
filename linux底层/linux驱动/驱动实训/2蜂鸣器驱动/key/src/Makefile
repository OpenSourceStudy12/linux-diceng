    KERNELDIR  ?=/mini2440/linux-2.6.32/include

all:key_beep_test
key_beep_test: key_beep_test.c
			arm-linux-gcc -I$(KERNELDIR) -o $@  $^
clean:
			rm -f  *~  key_beep_test.o  key_beep_test
