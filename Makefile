DEBUG_OPTIONS = -g -Wall
#DEBUG_OPTIONS = -Wall
OPTIMIZE = -O3
GUI_LDFLAGS =

MIN_LITE_BEAPI_VER = 0.0.3

TOPSRCDIR = $(shell pwd)

EXTRA_CFLAGS = -I $(TOPSRCDIR)
EXTRA_LDFLAGS =

CFLAGS = $(DEBUG_OPTIONS) $(OPTIMIZE) $(EXTRA_CFLAGS)
ifeq ($(findstring BePC,$(shell uname -a)),BePC)
CFLAGS += -Wno-multichar
LDFLAGS += -lbe -lroot -ltextencoding
else
CFLAGS += $(shell lite-beapi-config --cflags)
LDFLAGS = $(shell lite-beapi-config --libs)
endif

LIBSUNPINYIN_CFLAGS = $(shell pkg-config --cflags sunpinyin-2.0)
LIBSUNPINYIN_LIBS = $(shell pkg-config --libs sunpinyin-2.0)
CFLAGS += $(LIBSUNPINYIN_CFLAGS)
LDFLAGS += $(LIBSUNPINYIN_LIBS)

SO_CFLAGS = -fPIC
SO_SUFFIX = .so
SO_LDFLAGS = -shared -export-dynamic -L. -l:_APP_

ifeq ($(findstring MINGW32,$(shell uname -a)),MINGW32)
SO_CFLAGS =
SO_SUFFIX = .dll
SO_LDFLAGS = -shared -export-dynamic -L. -leime
#GUI_LDFLAGS = -mwindows
#CFLAGS += -pipe -march=i686
endif

CFLAGS += $(SO_CFLAGS)
CXXFLAGS = $(CFLAGS)

SUNPINYIN_OBJECTS =		\
	SunPinyinModule.o

TARGETS =					\
	SunPinyin$(SO_SUFFIX)

ifeq ($(findstring BePC,$(shell uname -a)),BePC)
all: build_targets
else
all:
	@if ! ( lite-beapi-config --atleast-version $(MIN_LITE_BEAPI_VER) ) ; then \
		echo "*** ERROR: requires Lite BeAPI >= $(MIN_LITE_BEAPI_VER) !!!"; \
	else \
		$(MAKE) build_targets; \
	fi
endif

build_targets: $(TARGETS)

SunPinyin$(SO_SUFFIX): $(SUNPINYIN_OBJECTS)
	g++ $(OPTIMIZE) $^ -o $@ $(LDFLAGS) $(SO_LDFLAGS)

clean:
	rm -f $(SUNPINYIN_OBJECTS)
	rm -f $(TARGETS)

install:

uninstall:

.SUFFIXES: .c .cpp .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

