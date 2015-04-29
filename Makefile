# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

SUBDIR=ats go misc

all:
	for subdir in $(SUBDIR); do \
            make -C $$subdir; \
	done

%.so : %.cc
	$(CXX) $(CXXFLAGS) -fPIC -pthread -shared $< -o $@

%: %.c
	$(CC) $< -o $@

.phony: clean $(SUBDIR)
clean:
	-rm -rf *~
	for subdir in $(SUBDIR); do \
            make clean -C $$subdir; \
        done
