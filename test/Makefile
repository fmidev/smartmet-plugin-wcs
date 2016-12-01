INCDIR = smartmet/plugins/$(SUBNAME)
TEST_TOP = $(shell pwd)
TOP = $(TEST_TOP)/..

TEST_DIRECTORIES = base
TEST_DEPENDENCIES = Wcs

# Compiler options

DEFINES = -DUNIX -D_REENTRANT

FLAGS = -std=c++11 -fPIC -Wall -W -Wno-unused-parameter -Wno-unknown-pragmas \
      -Wcast-align \
      -Wcast-qual \
      -Wno-inline \
      -Wno-multichar \
      -Wno-pmf-conversions \
      -Woverloaded-virtual  \
      -Wpointer-arith \
      -Wredundant-decls \
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

INCLUDES = -I$(includedir) \
	-I$(includedir)/smartmet


INCLUDES := -I$(TOP)/include $(INCLUDES)

obj/%.o : %.cpp ; @echo Compiling $<
	@mkdir -p obj
	$(CXX) $(CFLAGS) $(INCLUDES) -c -MD -MF $(patsubst obj/%.o, obj/%.d.new, $@) -o $@ $<
	@sed -e "s|^$(notdir $@):|$@:|" $(patsubst obj/%.o, obj/%.d.new, $@) >$(patsubst obj/%.o, obj/%.d, $@)
	@rm -f $(patsubst obj/%.o, obj/%.d.new, $@)



PLUGIN_TEST_SRCS = PluginTest.cpp
TEST_OBJS = $(patsubst %.cpp,obj/%.o,$(TEST_SRCS))
TEST_TARGETS = PluginTest

LIBS = -L$(libdir) \
	-lsmartmet-spine \
	-lsmartmet_newbase \
	-lsmartmet_macgyver \
	-lpqxx \
	-lboost_serialization \
	-lboost_date_time \
	-lboost_thread \
	-lboost_regex \
	-lboost_iostreams \
	-lboost_filesystem \
	-lboost_chrono \
	-lboost_system \
	-lxqilla \
	-lxerces-c \
	-lgdal \
	-lconfig++ \
	-lconfig \
	-lctpp2 \
	-lcurl \
	-lcrypto \
	-lbz2 -lz \
	-lstdc++ \
	-lpthread \
	-lm

all: $(TEST_TARGETS)

clean:
	rm -rf obj/*.o obj/*.d
	rm -vf $(TEST_TARGETS)
	for test_dir in $(TEST_DIRECTORIES) ; do $(MAKE) -C $$test_dir clean; done

test: $(TEST_TARGETS)
	@ok=true; \
	for test_dir in $(TEST_DIRECTORIES); do \
		if ! LD_LIBRARY_PATH=../../../spine:$(libdir) $(MAKE) -C $$test_dir $@ ; then ok=false; fi; \
	done; \
	$$ok;

$(TEST_TARGETS) : $(TEST_DEPENDENCIES)

PluginTest : obj/PluginTest.o ; @echo "Building $@"
	@$(CC) -o $@ $(TEST_CFLAGS) $(INCLUDES) $< -Ltestsuite -Wl,-rpath=$(libdir) $(LIBS)


Wcs:
	$(MAKE) -C $(TOP) -j 3 debug

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
