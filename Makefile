SUBNAME = wcs
SPEC = smartmet-plugin-$(SUBNAME)
INCDIR = smartmet/plugins/$(SUBNAME)
TOP = $(shell pwd)

# Compiler options

DEFINES = -DUNIX -D_REENTRANT

FLAGS = -std=c++11 -fPIC -Wall -W -Wno-unused-parameter \
	-fno-omit-frame-pointer \
	-Wno-unknown-pragmas \
      -Wcast-align \
      -Wcast-qual \
      -Wno-inline \
      -Wno-multichar \
      -Wno-pmf-conversions \
      -Wpointer-arith \
      -Wwrite-strings

# Compile options in detault, debug and profile modes

CFLAGS_RELEASE = $(DEFINES) $(FLAGS) -DNDEBUG -O2 -g
CFLAGS_DEBUG   = $(DEFINES) $(FLAGS) -Werror  -O0 -g

ARFLAGS = -r
DEFINES = -DUNIX -D_REENTRANT

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  override CFLAGS += $(CFLAGS_DEBUG)
else
  override CFLAGS += $(CFLAGS_RELEASE)
endif

# Installation directories

processor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  PREFIX = /usr
else
  PREFIX = $(PREFIX)
endif

ifeq ($(processor), x86_64)
  libdir = $(PREFIX)/lib64
else
  libdir = $(PREFIX)/lib
endif

bindir = $(PREFIX)/bin
includedir = $(PREFIX)/include
datadir = $(PREFIX)/share
enginedir = $(datadir)/smartmet/engines
plugindir = $(datadir)/smartmet/plugins
objdir = obj

#

# Boost 1.69

ifneq "$(wildcard /usr/include/boost169)" ""
  INCLUDES += -I/usr/include/boost169
  LIBS += -L/usr/lib64/boost169
endif

ifneq "$(wildcard /usr/gdal30/include)" ""
  INCLUDES += -I/usr/gdal30/include
  LIBS += -L/usr/gdal30/lib
else
  INCLUDES += -I/usr/include/gdal
endif


INCLUDES += -I$(includedir) \
	-I$(includedir)/jsoncpp \
	-I$(includedir)/smartmet \
	-I$(PREFIX)/gdal30/include

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
        -ljsoncpp \
        -lxqilla \
	-lxerces-c \
	-L$(PREFIX)/gdal30/lib -lgdal \
	-lconfig++ \
	-lconfig \
	-lctpp2 \
	-lcurl \
	-lcrypto \
	-lnetcdf_c++ \
	-lbz2 -lz \
	-lpthread \
	-lm

obj/%.o : %.cpp ; @echo Compiling $<
	@mkdir -p obj
	$(CXX) $(CFLAGS) $(INCLUDES) $(LIBS) -c -MD -MF $(patsubst obj/%.o, obj/%.d.new, $@) -o $@ $<
	@sed -e "s|^$(notdir $@):|$@:|" $(patsubst obj/%.o, obj/%.d.new, $@) >$(patsubst obj/%.o, obj/%.d, $@)
	@rm -f $(patsubst obj/%.o, obj/%.d.new, $@)


INCLUDES += -I$(TOP)/include \
	-I$(TOP)/include/request \
	-I$(TOP)/include/response \
	-I$(TOP)/include/xml \
	-isystem


# What to install

LIBFILE = $(SUBNAME).so

# How to install

INSTALL_PROG = install -p -m 775
INSTALL_DATA = install -p -m 664

# Compile option overrides

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_DEBUG)
endif

ifneq (,$(findstring profile,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_PROFILE)
endif

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
	$(CC) -shared -rdynamic $(CFLAGS) -o $(LIBFILE) $(OBJS) $(LIBS)

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
	rpmbuild -ta $(SPEC).tar.gz
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
	echo ./test/PluginTest.cpp >>files.list.new
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
	$(CC) $(CFLAGS) $(INCLUDES) -o /dev/null /tmp/$(SPEC).cpp $(LIBS); \
	done

cnf/templates/%.c2t: cnf/templates/%.template ; ( cd cnf/templates && $(PREFIX)/bin/ctpp2c $(notdir $<) $(notdir $@) )

.SUFFIXES: $(SUFFIXES) .cpp

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
