TARGET=sandbox
CC?=gcc
BUILD_DIR=./build
SRC_DIR=./src

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

CFLAGS=-O0 -g
LDLIBS=-lm\
			 -lSDL3

all: $(TARGET)

clean:
	rm -rf build/*

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LDLIBS) $^ -o $@

