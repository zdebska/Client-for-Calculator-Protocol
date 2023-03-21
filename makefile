CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LDFLAGS =

TARGET = ipkcpc

all: $(TARGET)

$(TARGET): ipkcpc.c
	$(CC) $(CFLAGS) ipkcpc.c -o $(TARGET)

run:
	./$(TARGET) -h <host> -p <port> -m <mode>

clean:
	rm -f $(TARGET)