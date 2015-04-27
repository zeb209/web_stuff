CC=gcc
CXX=g++

Target=epoll-example

all: $(Target)

%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
