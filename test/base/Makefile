TOP = $(shell pwd)/../../

XML_TEST_SRC = $(shell find xml -name '*.xml')
XML_POST_TESTS = $(subst :,\:,$(patsubst xml/%.xml, input/%.xml.post, $(XML_TEST_SRC)))

VALGRIND = valgrind \
	--num-callers=30 \
	--suppressions=../../../../data/valgrind/valgrind_libclntsh.supp \
	--suppressions=../../../../data/valgrind/valgrind_brainstorm.supp

TEST_RUNNER =

all:

clean:
	rm -f $(shell find input -type f -a -name '*.xml.post')
	rm -rf failures

test: WCS_PLUGIN := ../../../wcs.so
test-installed: WCS_PLUGIN := /usr/share/smartmet/plugins/wcs.so

test test-installed: $(XML_POST_TESTS) cnf/smartmet_test.conf
	@mkdir -p failures
	@$(TEST_RUNNER) smartmet-plugin-test --handler /wcs \
		--reactor-config cnf/smartmet_test.conf \
		--failures-dir failures \
		--timeout 300

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

cnf/smartmet_test.conf: cnf/smartmet_test.conf.in
	sed \
		-e "s:@WCS_SO@:$(WCS_PLUGIN):" \
		$< >$@

.PHONY: cnf/smartmet_test.conf
