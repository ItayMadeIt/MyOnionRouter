BUILD_DIR := $(shell pwd)/build

CLIENT_DIR := Client
RELAY_DIR  := Relay
SERVER_DIR := Server

DEBUG ?= 0

.PHONY: all clean client relay server

all: client relay server

client:
	$(MAKE) -C $(CLIENT_DIR) BUILD_DIR=$(BUILD_DIR)/client DEBUG=$(DEBUG)

relay:
	$(MAKE) -C $(RELAY_DIR)  BUILD_DIR=$(BUILD_DIR)/relay  DEBUG=$(DEBUG)

server:
	$(MAKE) -C $(SERVER_DIR) BUILD_DIR=$(BUILD_DIR)/server DEBUG=$(DEBUG)

clean:
	$(MAKE) -C $(CLIENT_DIR) clean BUILD_DIR=$(BUILD_DIR)/client
	$(MAKE) -C $(RELAY_DIR)  clean BUILD_DIR=$(BUILD_DIR)/relay
	$(MAKE) -C $(SERVER_DIR) clean BUILD_DIR=$(BUILD_DIR)/server
