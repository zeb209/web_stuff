JAVA=/usr/bin/java
AVRO=/home/bzeng/big-data/avro
AVRO_TOOLS=$(AVRO)/avro-tools-1.7.7.jar

.phony: validate

# validate a json against a schema
validate:user.frag user.fragjson

# json to frag. $(word 2,$^) is the second prerequisite.
%.frag:%.json %.avsc
	$(JAVA) -jar $(AVRO_TOOLS) jsontofrag --schema-file $(word 2,$^) $< > $@

# frag to json.
%.fragjson:%.frag %.avsc
	$(JAVA) -jar $(AVRO_TOOLS) fragtojson --schema-file $(word 2,$^) $< > $@

.phony:clean
clean:
	-rm -rf *~ *.frag *.fragjson
