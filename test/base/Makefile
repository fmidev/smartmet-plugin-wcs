TOP = $(shell pwd)/../../

XML_TEST_SRC = $(shell find xml -name '*.xml')
XML_POST_TESTS = $(subst :,\:,$(patsubst xml/%.xml, input/%.xml.post, $(XML_TEST_SRC)))

VALGRIND = valgrind \
	--num-callers=30 \
	--suppressions=../../../../data/valgrind/valgrind_libclntsh.supp \
	--suppressions=../../../../data/valgrind/valgrind_brainstorm.supp

all:

clean:
	rm -f $(shell find input -type f -a -name '*.xml.post')
	rm -rf failures

test:	../PluginTest $(XML_POST_TESTS)
	@mkdir -p failures
	@../PluginTest cnf/smartmet_test.conf

valgrind: ../PluginTest input-files
	@mkdir -p failures
	@$(VALGRIND) ../PluginTest cnf/smartmet_test.conf

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

../PluginTest:
	$(MAKE) -C .. PluginTest
