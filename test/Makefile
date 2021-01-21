INCDIR = smartmet/plugins/$(SUBNAME)
TEST_TOP = $(shell pwd)
TOP = $(TEST_TOP)/..

TEST_DIRECTORIES = base

all:

clean:
	for test_dir in $(TEST_DIRECTORIES) ; do $(MAKE) -C $$test_dir clean; done

test test-installed: all-templates
	@ok=true; \
	for test_dir in $(TEST_DIRECTORIES); do \
	    if ! $(MAKE) -C $$test_dir $@ ; then ok=false; fi; \
	done; \
	$$ok;

all-templates:
	$(MAKE) -C $(TOP) $@

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
