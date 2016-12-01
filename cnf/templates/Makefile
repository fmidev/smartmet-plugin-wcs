CTPP2C = ctpp2c

TEMPLATES = $(wildcard *.template)
COMPILED_TEMPLATES = $(patsubst %.template, %.c2t, $(TEMPLATES))

all:	$(COMPILED_TEMPLATES)

clean:
	rm -f *.c2t *~ include/*~
	rm -rf .deps

.template.c2t:
	$(CTPP2C) $< $@
	mkdir -p .deps
	echo $@ : $< $(shell cat $< | ./template_depend.pl) >.deps/$@.dep

.SUFFIXES: $(SUFFIXES) .template .c2t

ifneq ($(wildcard .deps/*.dep),)
include $(wildcard .deps/*.dep)
endif
