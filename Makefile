# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

SUBDIR=ats go misc

all: $(SUBDIR)

$(SUBDIR)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

%: %.c
	$(CC) $< -o $@

.phony: clean

clean: $(SUBDIR)
	-rm -rf *~
