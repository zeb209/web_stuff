# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

CC=gcc
CXX=g++

SUBDIR = protobuf
Target=epoll-example


all: $(Target)
	for subdir in $(SUBDIR); do \
            make -C $$subdir; \
	done


%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
	for subdir in $(SUBDIR); do \
            make clean -C $$subdir; \
        done
