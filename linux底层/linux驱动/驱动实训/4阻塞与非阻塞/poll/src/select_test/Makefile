EXEC=select_test
OBJS=select_test.o
SRC=select_test.c

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
