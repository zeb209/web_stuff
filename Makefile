SUBDIR=ats go misc

all:
	make -C ats
	make -C go
	make -C misc

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean $(SUBDIR)
clean:
	-rm -rf $(Target) *~
	make clean -C ats
	make clean -C go
	make clean -C misc
