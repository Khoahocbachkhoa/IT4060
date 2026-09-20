CC = gcc
CFLAGS = -O2

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

all : net_toolkit

net_toolkit : $(OBJ)
	$(CC) $(CFLAGS) -o net_toolkit $(OBJ)

clean:
	rm -f src/*.o net_toolkit downloaded_test.txt