CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = pop3_client
SRCS = pop3_client.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)