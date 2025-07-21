CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -m64 -I. -I$(INCLUDE_DIR) -pthread -D_POSIX_C_SOURCE=200112L 

SRC_DIRS = string_utils sock_utils server_cfg file_utils
SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

BUILD_DIR ?= build
LIB_DIR ?= build/lib

OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
LIB := $(LIB_DIR)/libutils.a

.PHONY: all clean

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
