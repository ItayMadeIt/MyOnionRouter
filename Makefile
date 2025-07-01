BUILD_DIR := $(shell pwd)/build

ENCRYPTIONS_DIR := encryptions
CLIENT_DIR := client
RELAY_DIR  := relay
SERVER_DIR := server

DEBUG ?= 0

.PHONY: all clean client relay server

all: encryptions client relay server

encryptions:
	$(MAKE) -C $(ENCRYPTIONS_DIR) BUILD_DIR=$(BUILD_DIR)/encryptions LIB_DIR=$(BUILD_DIR)/lib DEBUG=$(DEBUG)

client:
	$(MAKE) -C $(CLIENT_DIR) BUILD_DIR=$(BUILD_DIR)/client LIB_DIR=$(BUILD_DIR)/lib DEBUG=$(DEBUG)

relay:
	$(MAKE) -C $(RELAY_DIR)  BUILD_DIR=$(BUILD_DIR)/relay LIB_DIR=$(BUILD_DIR)/lib DEBUG=$(DEBUG)

server:
	$(MAKE) -C $(SERVER_DIR) BUILD_DIR=$(BUILD_DIR)/server LIB_DIR=$(BUILD_DIR)/lib DEBUG=$(DEBUG)

clean:
	$(MAKE) -C $(ENCRYPTIONS_DIR) clean BUILD_DIR=$(BUILD_DIR)/encryptions LIB_DIR=$(BUILD_DIR)/lib
	$(MAKE) -C $(CLIENT_DIR) clean BUILD_DIR=$(BUILD_DIR)/client LIB_DIR=$(BUILD_DIR)/lib
	$(MAKE) -C $(RELAY_DIR)  clean BUILD_DIR=$(BUILD_DIR)/relay LIB_DIR=$(BUILD_DIR)/lib
	$(MAKE) -C $(SERVER_DIR) clean BUILD_DIR=$(BUILD_DIR)/server LIB_DIR=$(BUILD_DIR)/lib
