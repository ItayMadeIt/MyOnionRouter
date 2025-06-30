CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I_include
SRC_DIRS = core net
SRC = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

LIB_DIR ?= build/lib
LIBS := $(wildcard $(LIB_DIR)/*.a)

BIN = $(BUILD_DIR)/relay

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: $(BIN)

# Rule to compile binary
$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) -o $@ $(LIBS)

# Rule to build object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
