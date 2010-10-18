DEBUG   = -O0 -ggdb -g3
DEBUG   = -O3
CFLAGS  = -Wall -pedantic -std=gnu99 $(DEBUG)
LDFLAGS = -lGL -lGLU -lglut
BINARY  = ray
VERSION = 0.1
PREFIX  = /usr/local
OBJECTS = $(BINARY).o

all: $(BINARY)

clean:
	rm -f $(BINARY).o $(BINARY)

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(BINARY) $(PREFIX)/bin/$(BINARY)
	chmod 755 $(PREFIX)/bin/$(BINARY)
	mkdir -p $(PREFIX)/man/man1
	sed "s/VERSION/$(VERSION)/g" < $(BINARY).1 > $(PREFIX)/man/man1/$(BINARY).1
	chmod 644 $(PREFIX)/man/man1/$(BINARY).1

uninstall:
	rm -f $(BINARY) $(PREFIX)/bin/$(BINARY)
