CC=gcc
CXX=g++

API=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/proxy/api/
TS=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/lib/ts
B_TS=/home/bzeng/web/ATS/LNKD-trafficserver/build/lib/ts

CXXFLAGS= -I$(API) -I$(TS) -I$(B_TS) -Wall -Werror -g

Target=TSContSchedule.so

all: $(Target)

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean
clean:
	-rm -rf $(Target) *~
