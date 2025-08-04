# === Config ===

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -Iinclude -O2
LDFLAGS := -lcurl -lcjson -lssl -lcrypto
SRC_DIR := src
INC_DIR := include
OBJ_DIR := build
BIN_DIR := bin

# === Files ===

TARGET = $(BIN_DIR)/lchk
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# === Targets ===

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
