EXEC=poll_test
OBJS=poll_test.o
SRC=poll_test.c

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
