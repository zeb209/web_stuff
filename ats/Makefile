# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

CC=gcc
CXX=g++
#CC=~/tools/gcc/install/bin/gcc
#CXX=~/tools/gcc/install/bin/g++

ATS_ROOT=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver
# ATS_ROOT=/home/bzeng/web/ATS/ats-upstream/trafficserver

API=$(ATS_ROOT)/proxy/api/
TS=$(ATS_ROOT)/lib/ts
B_TS=$(ATS_ROOT)/../build/lib/
ATS_LIB=$(ATS_ROOT)/lib

CXXFLAGS= -I$(API) -I$(TS) -I$(B_TS) -I$(ATS_LIB) -Wno-deprecated -Wall -Wno-deprecated -Wextra -pedantic -g

Target=TSContSchedule.so createTransactionCont.so StartCorpIpFetcher.so GetConfigOptions.so

all: $(Target)

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
