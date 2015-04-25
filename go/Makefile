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
