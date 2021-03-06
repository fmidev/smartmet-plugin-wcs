SUBNAME = wcs
SPEC = smartmet-plugin-$(SUBNAME)
INCDIR = smartmet/plugins/$(SUBNAME)
TOP = $(shell pwd)

REQUIRES = configpp ctpp2 gdal jsoncpp

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

DEFINES = -DUNIX -D_REENTRANT
ARFLAGS = -r

INCLUDES += \
	-I$(includedir)/smartmet \

LIBS += -L$(libdir) \
	-lsmartmet-spine \
	-lsmartmet-newbase \
	-lsmartmet-macgyver \
	-lboost_date_time \
        -lboost_serialization \
	-lboost_thread \
	-lboost_regex \
	-lboost_iostreams \
	-lboost_filesystem \
        -lboost_chrono \
	-lboost_system \
        -lxqilla \
	-lxerces-c \
	$(REQUIRED_LIBS) \
	-lcurl \
	-lcrypto \
	-lnetcdf_c++ \
	-lbz2 -lz \
	-lpthread \
	-lm

obj/%.o : %.cpp ; @echo Compiling $<
	@mkdir -p obj
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d.new, $@) -o $@ $<
	@sed -e "s|^$(notdir $@):|$@:|" $(patsubst obj/%.o, obj/%.d.new, $@) >$(patsubst obj/%.o, obj/%.d, $@)
	@rm -f $(patsubst obj/%.o, obj/%.d.new, $@)


INCLUDES += -I$(TOP)/include \
	-I$(TOP)/include/request \
	-I$(TOP)/include/response \
	-I$(TOP)/include/xml

# What to install

LIBFILE = $(SUBNAME).so

# Compilation directories

vpath %.cpp source source/request source/response source/xml
vpath %.h include include/request include/response include/xml

# The files to be compiled
SRCS = $(wildcard source/*.cpp) $(wildcard source/request/*.cpp) $(wildcard source/response/*.cpp) $(wildcard source/xml/*.cpp)
HDRS = $(wildcard include/*.h) $(wildcard include/request/*.h) $(wildcard include/response/*.h) $(wildcard include/xml/*.h)
OBJS = $(patsubst %.cpp, obj/%.o, $(notdir $(SRCS)))

TEMPLATES = $(wildcard cnf/templates/*.template)
COMPILED_TEMPLATES = $(patsubst %.template, %.c2t, $(TEMPLATES))

.PHONY: test rpm

# The rules

all: objdir $(LIBFILE) all-templates
debug: all
release: all
profile: all

$(LIBFILE): $(OBJS)
	$(CC) -shared -rdynamic $(LDFLAGS) -o $(LIBFILE) $(OBJS) $(LIBS)

clean:	clean-templates
	rm -f $(LIBFILE) obj/*.o obj/*.d *~ source/*~ include/*~ cnf/templates/*.c2t
	rm -f files.list files.tmp
	$(MAKE) -C test $@

format:
	clang-format -i -style=file include/*.h include/*/*.h source/*.cpp source/*/*.cpp test/*.cpp

install:
	@mkdir -p $(plugindir)
	$(INSTALL_PROG) $(LIBFILE) $(plugindir)/$(LIBFILE)

# Separate depend target is no more needed as dependencies are updated automatically
# and are always up to time
depend:

test test-installed:
	$(MAKE) -C test $@

all-templates:
	$(MAKE) -C cnf/templates all

clean-templates:
	$(MAKE) -C cnf/templates clean

html::
	mkdir -p /data/local/html/lib/$(SPEC)
	doxygen $(SPEC).dox

objdir:
	@mkdir -p $(objdir)

rpm: clean file-list $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz \
		--transform "s,^,plugins/$(SPEC)/," $(shell cat files.list)
	rpmbuild -tb $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

file-list:	cnf/XMLGrammarPool.dump
	find . -name '.gitignore' >files.list.new
	find . -name 'Makefile' -o -name '*.spec' >>files.list.new
	find source -name '*.h' -o -name '*.cpp' >>files.list.new
	find include -name '*.h' >>files.list.new
	find tools -name '*.h' -o -name '*.cpp' >>files.list.new
	find cnf -name '*.template' >>files.list.new
	echo cnf/templates/template_depend.pl >>files.list.new
	echo cnf/XMLGrammarPool.dump >>files.list.new
	echo cnf/XMLSchemas.cache >>files.list.new
	find test/base -name '*.conf' >>files.list.new
	find test/base/input -name '*.get' >>files.list.new
	find test/base/output -name '*.get' -o -name '*.kvp.post' -o -name '*.xml.post' >>files.list.new
	find test/base/xml -name '*.xml' >>files.list.new
	find test -name '*.pl' >>files.list.new
	find test -name '*.cpp' >>files.list.new
	find tools/xml_samples -name '*.xml' >>files.list.new
	cat files.list.new | sed -e 's:^\./::' | sort | uniq >files.list
	rm -f files.list.new

file-list-test: file-list
	git ls-files . | sort >files.tmp
	diff -u files.list files.tmp
	rm -f files.tmp

headertest:
	@echo "Checking self-sufficiency of each header:"
	@echo
	@for hdr in $(HDRS); do \
	echo $$hdr; \
	echo "#include \"$$hdr\"" > /tmp/$(SPEC).cpp; \
	echo "int main() { return 0; }" >> /tmp/$(SPEC).cpp; \
	$(CXX) $(CFLAGS) $(INCLUDES) -o /dev/null /tmp/$(SPEC).cpp; \
	done

cnf/templates/%.c2t: cnf/templates/%.template ; ( cd cnf/templates && $(PREFIX)/bin/ctpp2c $(notdir $<) $(notdir $@) )

.SUFFIXES: $(SUFFIXES) .cpp

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
