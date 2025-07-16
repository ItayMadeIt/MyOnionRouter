CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -m64 -I_include -I$(INCLUDE_DIR) 
SRC_DIRS = core

SRC = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

LIB_DIR ?= build/lib
INCLUDE_DIR ?= ./_include
LIBS = -L$(LIB_DIR) -lencryptions -lgmp -lutils

BIN = $(BUILD_DIR)/client

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: $(BIN)

$(BIN): $(OBJ)
	@echo "Compiling binary $< to $@"
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $< to $@"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "rm -rf $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

