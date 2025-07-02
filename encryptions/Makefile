CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -I.
SRC_DIRS = aes dh sha

SRC = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
BUILD_DIR ?= build
LIB_DIR ?= build/lib

OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

$(info BUILD_DIR=$(BUILD_DIR))
$(info LIB_DIR=$(LIB_DIR))
$(info OBJ=$(OBJ))

LIB = $(LIB_DIR)/libencryptions.a

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
	$(shell rm -rf $(BUILD_DIR))
	$(shell rm -rf $(LIB_DIR))
