CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -I.

SRC_DIRS = aes dh sha source
SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

BUILD_DIR ?= build
LIB_DIR ?= build/lib

OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
LIB := $(LIB_DIR)/libencryptions.a

.PHONY: all clean

$(info BUILD_DIR=$(BUILD_DIR))
$(info SRC=$(SRC))
$(info OBJ=$(OBJ))

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: $(LIB)

$(LIB): $(OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning: $(BUILD_DIR), $(LIB_DIR)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(LIB_DIR)
