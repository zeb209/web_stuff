# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

CC=~/tools/gcc/install/bin/gcc
CXX=~/tools/gcc/install/bin/g++

# API=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/proxy/api/
# TS=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/lib/ts
# B_TS=/home/bzeng/web/ATS/LNKD-trafficserver/build/lib/ts

API=/home/bzeng/web/ATS/ats-upstream/trafficserver/proxy/api/
TS=/home/bzeng/web/ATS/ats-upstream/trafficserver/lib/ts
B_TS=/home/bzeng/web/ATS/ats-upstream/build/lib/ts

CXXFLAGS= -I$(API) -I$(TS) -I$(B_TS) -Wall -Werror -Wno-deprecated -Wextra -pedantic -g -std=c++11

Target=TSContSchedule.so createTransactionCont.so StartCorpIpFetcher.so

all: $(Target)

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
