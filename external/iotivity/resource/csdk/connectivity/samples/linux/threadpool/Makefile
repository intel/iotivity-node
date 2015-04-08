CC=gcc
ROOT_DIR = ../../..
INC_DIR := -I$(ROOT_DIR)/api -I$(ROOT_DIR)/inc -I$(ROOT_DIR)/common/inc
SRC_DIR := $(ROOT_DIR)/common/src/
CFLAGS = -Wall
COMPILEFLAGS = `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0` 

OUT = threadpool

all: 
	$(CC) $(CFLAGS) $(INC_DIR) $(COMPILEFLAGS) $(SRC_DIR)uthreadpool.c $(SRC_DIR)umutex.c $(SRC_DIR)/../logger.c main.c -o $(OUT) $(LDFLAGS)
clean:
	rm -f $(OUT)
