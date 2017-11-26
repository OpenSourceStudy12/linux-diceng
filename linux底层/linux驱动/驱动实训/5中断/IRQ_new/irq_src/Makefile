EXEC=pan_irq_test
OBJS=pan_irq_test.o
SRC=pan_irq_test.c

CC=arm-linux-gcc
CFLAGS+=-O2 -Wall
LDFLAGS+=

all:$(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -vf $(EXEC) *.o *~
