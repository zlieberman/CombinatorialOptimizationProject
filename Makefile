CFLAGS  = -g -Wall
TARGET = main
CC = g++

all: $(TARGET)

$(TARGET): $(TARGET).cpp utils.hpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)