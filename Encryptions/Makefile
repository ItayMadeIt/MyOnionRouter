CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -std=c11 -I_include
SRC_DIRS = aes rsa sha

SRC = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

LIB = $(LIB_DIR)/lib_encryptions.a

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -g
else
	CFLAGS += -O2
endif

all: $(LIB)

# Build the static library
$(LIB): $(OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

# Compile source files to object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
