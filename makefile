CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LDFLAGS =

ifeq ($(OS),Windows_NT)
	# Windows settings
	EXECUTABLE = ipkcpc.exe
else
	# Linux settings
	EXECUTABLE = ipkcpc
endif

SRC = ipkcpc.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJ)