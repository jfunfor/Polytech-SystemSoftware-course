# === Config ===
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2
LDFLAGS =

# === Project Files ===
SRCS = main.c logic.c
OBJS = build/main.o build/logic.o
HEADERS = lchk.h
TARGET = lchk

# === Installation paths ===
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

build/%.o: %.c 
		@mkdir -p build
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -rf build $(TARGET)

install: $(TARGET)
		install -d $(BINDIR)
		install -m 755 $(TARGET) $(BINDIR)

uninstall:
		rm -f $(BINDIR)/$(TARGET)

