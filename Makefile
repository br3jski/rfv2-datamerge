# Makefile

# Variables
CC = gcc
CFLAGS = -Wall -Werror
TARGET = myprogram

# Targets
all: $(TARGET)

$(TARGET): main.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

utils.o: utils.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
