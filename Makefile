TARGET=mkdfu

CC=gcc
CFLAGS= -c -Wall
LDFLAGS=

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
LIBRARY=cjson
.PHONY: clean

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) -l$(LIBRARY) -o $@
$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 
