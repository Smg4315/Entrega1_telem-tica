# Build del servidor NMP y de las pruebas unitarias (requiere entorno POSIX: Linux/WSL).
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -pthread

BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj

# Todos los .c bajo src/ (src/main.c y src/<modulo>/*.c).
SRC  = $(wildcard src/*.c src/*/*.c)
OBJ  = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
HDR  = $(wildcard include/*.h)

# Las pruebas enlazan protocolo + estado en memoria (sin sockets ni main.c).
PROTOCOL_SRC = $(wildcard src/protocol/*.c) src/core/node_registry.c src/core/state_manager.c
TEST_SRC     = $(wildcard tests/test_nmp_*.c)
TEST_BIN     = $(patsubst tests/%.c,$(BIN_DIR)/%,$(TEST_SRC))

.PHONY: all test clean

all: $(BIN_DIR)/server

$(BIN_DIR)/server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c $(HDR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/test_%: tests/test_%.c $(PROTOCOL_SRC) $(HDR)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) $(LDFLAGS)

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "== $$t"; ./$$t || exit 1; done

clean:
	rm -rf $(BIN_DIR)
