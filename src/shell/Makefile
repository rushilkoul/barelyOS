CC     := gcc
CFLAGS := -Wall -Wextra -O2
OUT    := ../../rootfs-overlay/bin/barelysh

.PHONY: all clean

all: $(OUT)

$(OUT): shell.c shell.h
	$(CC) $(CFLAGS) -o $@ shell.c

clean:
	rm -f $(OUT)
