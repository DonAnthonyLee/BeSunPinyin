DEBUG_OPTIONS = -g -Wall
#DEBUG_OPTIONS = -Wall
OPTIMIZE = -O3

MIN_LITE_BEAPI_VER = 0.0.4

ifeq ($(findstring MINGW32,$(shell uname -a)),MINGW32)
HOST := MINGW32
else
HOST := $(shell uname)
endif

IS_BEOS_PLATFORM = 0
ifeq ($(HOST),Haiku)
IS_BEOS_PLATFORM = 1
else
ifeq ($(HOST),BeOS)
IS_BEOS_PLATFORM = 1
endif
endif

ifeq ($(IS_BEOS_PLATFORM),1)
PREFIX := /boot/home
PKG_CONFIG_PATH := /boot/develop/lib/pkgconfig
else
ifeq ($(HOST),MINGW32)
PREFIX := /MinGW
PKG_CONFIG_PATH := $(PREFIX)/lib/pkgconfig
else
ifeq ($(HOST),Darwin)
#PREFIX := /usr/local
#PKG_CONFIG_PATH := $(PREFIX)/lib/pkgconfig
else
PREFIX := /usr
PKG_CONFIG_PATH := $(PREFIX)/lib/pkgconfig
endif
endif
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
DLLTOOL := $(CROSS_COMPILE)dlltool
LITE_BEAPI_CONFIG := $(CROSS_COMPILE)lite-beapi-config
PKG_CONFIG := $(CROSS_COMPILE)pkg-config

TOPSRCDIR = $(shell pwd)

EXTRA_CFLAGS = -I $(TOPSRCDIR)
EXTRA_LDFLAGS =

CFLAGS = $(DEBUG_OPTIONS) $(OPTIMIZE) $(EXTRA_CFLAGS) $(CPPFLAGS)
ifeq ($(IS_BEOS_PLATFORM),1)
CFLAGS += -Wno-multichar
LDFLAGS += -lbe -lroot -ltextencoding
else
CFLAGS += $(shell PREFIX=$(PREFIX) CROSS_COMPILE=$(CROSS_COMPILE) $(LITE_BEAPI_CONFIG) --cflags)
LDFLAGS = $(shell PREFIX=$(PREFIX) CROSS_COMPILE=$(CROSS_COMPILE) $(LITE_BEAPI_CONFIG) --libs)
endif

LIBSUNPINYIN_CFLAGS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --cflags sunpinyin-2.0)
LIBSUNPINYIN_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --libs sunpinyin-2.0)
CFLAGS += $(LIBSUNPINYIN_CFLAGS)
LDFLAGS += $(LIBSUNPINYIN_LIBS)

ifeq ($(HOST),MINGW32)
SO_CFLAGS =
SO_SUFFIX = .dll
SO_LDFLAGS = -shared -leime
else
ifeq ($(HOST),Darwin)
SO_CFLAGS =
SO_SUFFIX = .dylib
SO_LDFLAGS = -shared -leime
else
ifeq ($(IS_BEOS_PLATFORM),1)
SO_CFLAGS = -fPIC
SO_SUFFIX = .so
SO_LDFLAGS = -shared -export-dynamic -L. -l:_APP_
SO_DEPENDS = _APP_
else
SO_CFLAGS = -fPIC
SO_SUFFIX = .so
# binutils >= 2.35 cannot use executable file as input to a link
SO_LDFLAGS = -shared -export-dynamic -leime
endif
endif
endif

CFLAGS += $(SO_CFLAGS)
CXXFLAGS = $(CFLAGS)

SUNPINYIN_OBJECTS =			\
	SunPinyinStatusWindow.lo	\
	SunPinyinHandler.lo		\
	SunPinyinModule.lo

TARGETS =			\
	SunPinyin$(SO_SUFFIX)

ifeq ($(IS_BEOS_PLATFORM),1)
all:
	@if ! ( PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --exists sunpinyin-2.0 ) ; then \
		echo "*** ERROR: requires libsunpinyin-2.0"; \
	else \
		$(MAKE) build_targets; \
	fi
else
all:
	@if ! ( PREFIX=$(PREFIX) CROSS_COMPILE=$(CROSS_COMPILE) $(LITE_BEAPI_CONFIG) --atleast-version $(MIN_LITE_BEAPI_VER) ) ; then \
		echo "*** ERROR: requires Lite BeAPI >= $(MIN_LITE_BEAPI_VER) !!!"; \
	else \
		if ! ( PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --exists sunpinyin-2.0 ) ; then \
			echo "*** ERROR: requires libsunpinyin-2.0"; \
		else \
			$(MAKE) build_targets; \
		fi; \
	fi
endif

build_targets: $(TARGETS)

ifeq ($(IS_BEOS_PLATFORM),1)
ifeq ($(HOST),Haiku)
_APP_:
	ln -s /boot/system/servers/input_server _APP_
else
_APP_:
	ln -s /boot/beos/system/servers/input_server _APP_
endif
endif

SunPinyin$(SO_SUFFIX): $(SUNPINYIN_OBJECTS) $(SO_DEPENDS)
	$(CXX) $(OPTIMIZE) $(SUNPINYIN_OBJECTS) -o $@ $(LDFLAGS) $(SO_LDFLAGS)

clean:
	rm -f $(SUNPINYIN_OBJECTS)
	rm -f $(TARGETS)

install:

uninstall:

.SUFFIXES: .c .cpp .lo
.c.lo:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.lo:
	$(CXX) $(CXXFLAGS) -c $< -o $@

