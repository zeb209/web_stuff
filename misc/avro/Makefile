# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

AVROGEN=avrogencpp
NAMESPACE=c
CXX=g++
CFLAGS=-g -Wall -Werror # -Wextra # -pedantic

AVRO_ROOT=/home/bzeng/big-data/avro/install
AVRO_INCLUDE=$(AVRO_ROOT)/include
AVRO_LIB=$(AVRO_ROOT)/lib

Target=generated load-schema custom

all:$(Target)

generated: generated.cc cpx.hh
custom: custom.cc cpx.hh imaginary.hh

%: %.cc
	$(CXX) $(CFLAGS) -I$(AVRO_INCLUDE) -L$(AVRO_LIB) -lavrocpp -Wl,-rpath,$(AVRO_LIB) $< -o $@

imaginary.hh:imaginary.json
	$(AVROGEN) -i $< -o $@ -n i

%.hh: %.json
	$(AVROGEN) -i $< -o $@ -n c

.phony: clean
clean:
	-rm -rf *~ $(Target) cpx.hh imaginary.hh test.bin
