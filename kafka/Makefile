# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

API=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/proxy/api/
TS=/home/bzeng/web/ATS/LNKD-trafficserver/li-trafficserver/lib/ts
B_TS=/home/bzeng/web/ATS/LNKD-trafficserver/build/lib/ts

KAFKA_PATH=/home/bzeng/web/ATS_plugins/LNKD-trafficserver-plugin-userrequestevent_trunk/kafka/libkafka
LIKAFKA_PATH=/home/bzeng/web/ATS_plugins/LNKD-trafficserver-plugin-userrequestevent_trunk/kafka/liblikafka

CXX=g++
CXXFLAGS= -I$(API) -I$(TS) -I$(B_TS) -I$(KAFKA_PATH)/src/include -I$(LIKAFKA_PATH)/src/include \
  -std=c++0x -Wall -Werror -g -Wno-deprecated -Wextra -pedantic

Target = RequestEvent.h RequestEventSchema.h KafkaProducer.so

all: $(Target)

# Generator the cpp file from avro. the sed line removes the last comma of a enum member.
# If you specify -std=c++0x without -pedantic, the compiler is ok. double $$ is needed
# because bash expands $!.
%.h : %.avsc
	avrogencpp -i $< -o $@
	sed -i ':a;N;$$!ba;s/,\n};/\n};/g' $@

%.so: %.cc KafkaProducer.h KafkaTopicSchema.h RequestEvent.h $(LIKAFKA_PATH)/.libs/liblikafka.a $(KAFKA_PATH)/.libs/libkafka.a
	$(CXX) $(CXXFLAGS) -g -fPIC -pthread -shared $< $(LIKAFKA_PATH)/.libs/liblikafka.a $(KAFKA_PATH)/.libs/libkafka.a -lavrocpp -lcurl -lli_atscppapi -o $@

.phony: clean
clean:
	-rm -rf *~ $(Target)
