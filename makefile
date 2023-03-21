CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LDFLAGS =

TARGET = ipkcpc
run: 
	$(CC) $(CFLAGS) ipkcpc.c -o $(TARGET)

clean:
	rm -f $(TARGET)