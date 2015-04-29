# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

GO=/usr/bin/go

GCFLAGS=-gcflags "-N -l"
# "-ldflags -w" omits the debug info for go

Target=originServer sendRequest

all: $(Target)

%: %.go
	$(GO) build $(GCFLAGS) $<

.phony: clean
clean:
	-rm -rf $(Target) *~
