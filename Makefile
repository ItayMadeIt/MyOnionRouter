PWD_DIR = $(shell pwd)
BUILD_DIR := build

ENCRYPTIONS_DIR := encryptions
UTILS_DIR := utils
CLIENT_DIR := client
RELAY_DIR  := relay
SERVER_DIR := server

LIB_DIR := library/lib
INCLUDE_DIR := library/include

DEBUG ?= 0

.PHONY: all clean encryptions utils client relay server

all: encryptions utils client relay server

encryptions:
	$(MAKE) -C $(ENCRYPTIONS_DIR) \
		BUILD_DIR=../$(BUILD_DIR)/encryptions \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

utils:
	$(MAKE) -C $(UTILS_DIR) \
		BUILD_DIR=../$(BUILD_DIR)/utils \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

client:
	$(MAKE) -C $(CLIENT_DIR) \
		BUILD_DIR=../$(BUILD_DIR)/client \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

relay:
	$(MAKE) -C $(RELAY_DIR) \
		BUILD_DIR=../$(BUILD_DIR)/relay \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

server:
	$(MAKE) -C $(SERVER_DIR) \
		BUILD_DIR=../$(BUILD_DIR)/server \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

clean:
	$(MAKE) -C $(ENCRYPTIONS_DIR) clean \
		BUILD_DIR=../$(BUILD_DIR)/encryptions \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

	$(MAKE) -C $(UTILS_DIR) clean \
		BUILD_DIR=../$(BUILD_DIR)/utils \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

	$(MAKE) -C $(CLIENT_DIR) clean \
		BUILD_DIR=../$(BUILD_DIR)/client \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

	$(MAKE) -C $(RELAY_DIR) clean \
		BUILD_DIR=../$(BUILD_DIR)/relay \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)
		
	$(MAKE) -C $(SERVER_DIR) clean \
		BUILD_DIR=../$(BUILD_DIR)/server \
		LIB_DIR=../$(LIB_DIR) \
		INCLUDE_DIR=../$(INCLUDE_DIR) \
		DEBUG=$(DEBUG)

