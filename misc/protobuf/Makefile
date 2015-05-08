# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

CC=gcc
CXX=g++

CXXFLAGS=-Werror -Wall -Wextra -pedantic -g
Target = ProtobufWriter

all: $(Target)

ProtobufWriter : ProtobufWriter.cc
	$(CXX) $(CXXFLAGS) $< -ltrafficserver-protobuf -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
