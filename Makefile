CC	= gcc
CFLAGS	=
LFLAGS	= -lpthread

all: chat

ifeq ($(DEBUG), 1)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O3
endif


chat: main.o server.o client.o
	$(CC) $(CFLAGS) -o chat main.o server.o client.o $(LFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

.PHONY: clean

clean:
	rm -f ngsobel *.o *.a *.gch
