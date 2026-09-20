CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = smtp_client
SRCS = smtp_client.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)