INCDIR = smartmet/plugins/$(SUBNAME)
TEST_TOP = $(shell pwd)
TOP = $(TEST_TOP)/..

TEST_DIRECTORIES = base

all:

clean:
	for test_dir in $(TEST_DIRECTORIES) ; do $(MAKE) -C $$test_dir clean; done

test: Wcs
	@ok=true; \
	for test_dir in $(TEST_DIRECTORIES); do \
	    if ! $(MAKE) -C $$test_dir $@ ; then ok=false; fi; \
	done; \
	$$ok;

Wcs:
	$(MAKE) -C $(TOP) -j 3 debug

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
