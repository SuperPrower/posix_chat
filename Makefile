CC	= gcc
CFLAGS	=
LFLAGS	= -lpthread

all: objdir chat

ifeq ($(DEBUG), 1)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O3
endif

.PHONY: objdir

objdir:
	mkdir -p obj

chat: obj/main.o obj/server.o obj/client.o
	$(CC) $(CFLAGS) -o chat obj/main.o obj/server.o obj/client.o $(LFLAGS)

obj/main.o: objdir src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/server.o: objdir src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o obj/server.o

obj/client.o: objdir src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o obj/client.o

.PHONY: clean

clean:
	rm -f chat obj/*.o *.a *.gch
