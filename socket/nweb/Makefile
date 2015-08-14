CC=gcc
CFLAGS=-Wall -Werror -Wextra -pedantic -g

Target=nweb23 client

all:$(Target)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
