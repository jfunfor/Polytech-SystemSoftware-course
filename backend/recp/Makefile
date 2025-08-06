# === Config ===

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2
LDFLAGS =#-Wl,-z,now -Wl,-z,relro

PROJECT = recp
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# === Targets ===

all: recp

recp: recp.o
	 $(CC) $(CFLAGS) $(LDFLAGS) -o recp recp.o

recp.o: recp.c
	$(CC) $(CFLAGS) -c recp.c -o recp.o

clean:
	rm recp recp.o

install: recp
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(PROJECT) $(DESTDIR)$(BINDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PROJECT)

.PHONY: all clean
