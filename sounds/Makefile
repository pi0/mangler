SOURCE_RAW=channelenter.raw channelleave.raw login.raw logout.raw talkend.raw talkstart.raw
OBJECT_RAW=$(filter %.h,$(SOURCE_RAW:.raw=.h))

all: mangler-sounds.h

bin2h: bin2h.o

mangler-sounds.h: $(OBJECT_RAW)
	cat $(OBJECT_RAW) > $@

%.h: %.raw bin2h
	./bin2h $< $@ $(shell echo $@ | sed -e 's/^/sound\_/' -e 's/\..*$$//')

clean:
	rm -f mangler-sounds.h $(OBJECT_RAW) *.o

distclean: clean
	rm -f bin2h

install: mangler-sounds.h

uninstall:
