EXEC=timekey_test
OBJS=timekey_test.o
SRC=timekey_test.c

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
