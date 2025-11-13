# Makefile para Sistema de Archivos Simple

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = filesystem
SOURCE = filesystem.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean
