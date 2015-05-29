JAVA=/usr/bin/java
AVRO=/home/bzeng/big-data/avro
AVRO_TOOLS=$(AVRO)/avro-tools-1.7.7.jar

Target=twitter.avro twitter-snappy.avsc

all: pre-build $(Target)

pre-build:
	cp twitter.avsc twitter-snappy.avsc

# convert a json file into binary avro with a schema
%.avro: %.json
	$(JAVA) -jar $(AVRO_TOOLS) fromjson --codec snappy --schema-file twitter.avsc $< > $@

# convert a binary avro to json.
%.json: %.avro
	$(JAVA) -jar $(AVRO_TOOLS) tojson $< > $@

# retrieve avro schema from binary avro.
%.avsc: %.avro
	$(JAVA) -jar $(AVRO_TOOLS) getschema $< > $@

# validate a json against a schema by using jsontofrag and fragtojson.
.phony:validate
validate: twitter.frag twitter.fragjson
	-rm $+
# cat twitter.json | java -jar ~/big-data/avro/avro-tools-1.7.7.jar jsontofrag "`cat twitter.avsc`" - | java -jar ~/big-data/avro/avro-tools-1.7.7.jar fragtojson "`cat twitter.avsc`" -

# json to frag. $(word 2,$^) is the second prerequisite.
%.frag:%.json %.avsc
	$(JAVA) -jar $(AVRO_TOOLS) jsontofrag --schema-file $(word 2,$^) $< > $@

# frag to json.
%.fragjson:%.frag %.avsc
	$(JAVA) -jar $(AVRO_TOOLS) fragtojson --schema-file $(word 2,$^) $< > $@

.phony:clean
clean:
	-rm -rf *~ $(Target) twitter-snappy.avro
