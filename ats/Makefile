# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

CC=gcc
CXX=g++

API=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/proxy/api/
TS=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/lib/ts
B_TS=/home/bzeng/web/ATS/LNKD-trafficserver/build/lib/ts

CXXFLAGS= -I$(API) -I$(TS) -I$(B_TS) -Wall -Werror -g -Wno-deprecated -Wextra

Target=TSContSchedule.so createTransactionCont.so

all: $(Target)

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
