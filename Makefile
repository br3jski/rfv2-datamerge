# Define the compiler
CC = gcc

# Compiler flags
CFLAGS  = -g -Wall

# The build target executable:
TARGET = server_app

# Define source files
SOURCES = src/main.c src/data.c src/threadpool.c src/network.c src/thread.c
# Define object files
OBJECTS = $(SOURCES:.c=.o)

# Define include directory
CFLAGS += -Isrc

# Linking with pthread library
LDFLAGS = -pthread

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

# To obtain object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJECTS) src/*~ *~
