CC	= gcc
CFLAGS	=
LFLAGS	= -lncurses

BINARY = posix_chat

VERSION = 0.1

all: objdir $(BINARY)

ifeq ($(DEBUG), 1)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O3
endif

.PHONY: objdir

objdir:
	mkdir -p obj

$(BINARY): obj/main.o obj/server.o obj/client.o
	$(CC) $(CFLAGS) -o $(BINARY) obj/main.o obj/server.o obj/client.o $(LFLAGS)

obj/main.o: objdir src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/server.o: objdir src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o obj/server.o

obj/client.o: objdir src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o obj/client.o

tarball:
	mkdir -p $(BINARY)-$(VERSION)
	cp -R src/ Makefile LICENSE README.md $(BINARY)-$(VERSION)/
	tar -czf $(BINARY)-$(VERSION).tar.gz $(BINARY)-$(VERSION)

install: all
	mkdir -p $(DESTDIR)/usr/bin
	install -m 755 $(BINARY) $(DESTDIR)/usr/bin

.PHONY: clean

clean:
	rm -f $(BINARY) obj/*.o *.a *.gch
