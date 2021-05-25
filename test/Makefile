DEBUG_OPTIONS = -g -Wall
#DEBUG_OPTIONS = -Wall
OPTIMIZE = -O3

MIN_LITE_BEAPI_VER = 0.0.3

ifeq ($(findstring MINGW32,$(shell uname -a)),MINGW32)
HOST := MINGW32
else
HOST := $(shell uname)
endif

ifeq ($(HOST),MINGW32)
EXEC_SUFFIX = .exe
else
EXEC_SUFFIX =
endif

SINGLE_C_FILE_PROGRAMS =
SINGLE_CXX_FILE_PROGRAMS = Standalone
SINGLE_FILE_PROGRAMS = $(SINGLE_C_FILE_PROGRAMS) $(SINGLE_CXX_FILE_PROGRAMS)

TARGET_1 =
TARGET_2 =
TARGET_3 =

OBJECTS_1 =
OBJECTS_2 =
OBJECTS_3 =

TARGETS = $(TARGET_1) $(TARGET_2) $(TARGET_3)
OBJECTS = $(OBJECTS_1) $(OBJECTS_2) $(OBJECTS_3)

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
PREFIX := /usr
PKG_CONFIG_PATH := $(PREFIX)/lib/pkgconfig
endif
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
LITE_BEAPI_CONFIG := $(CROSS_COMPILE)lite-beapi-config
PKG_CONFIG := $(CROSS_COMPILE)pkg-config

CFLAGS = $(DEBUG_OPTIONS) $(OPTIMIZE)
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

$(C_PROGRAM)$(EXEC_SUFFIX): $(C_PROGRAM).o
	$(CC) $(DEBUG_OPTIONS) $(OPTIMIZE) $^ -o $@ $(LDFLAGS)

$(CXX_PROGRAM)$(EXEC_SUFFIX): $(CXX_PROGRAM).o
	$(CXX) $(DEBUG_OPTIONS) $(OPTIMIZE) $^ -o $@ $(LDFLAGS)

build_targets:
	for f in $(SINGLE_C_FILE_PROGRAMS) .none; do \
		(test $$f = .none || $(MAKE) C_PROGRAM=$$f $$f$(EXEC_SUFFIX)) \
	done
	for f in $(SINGLE_CXX_FILE_PROGRAMS) .none; do \
		(test $$f = .none || $(MAKE) CXX_PROGRAM=$$f $$f$(EXEC_SUFFIX)) \
	done
	for f in $(TARGETS) .none; do \
		(test $$f = .none || $(MAKE) $$f) \
	done

$(TARGET_1): $(OBJECTS_1)
	$(CXX) $(DEBUG_OPTIONS) $(OPTIMIZE) $^ -o $@ $(LDFLAGS)

$(TARGET_2): $(OBJECTS_2)
	$(CXX) $(DEBUG_OPTIONS) $(OPTIMIZE) $^ -o $@ $(LDFLAGS)

$(TARGET_3): $(OBJECTS_3)
	$(CXX) $(DEBUG_OPTIONS) $(OPTIMIZE) $^ -o $@ $(LDFLAGS)

clean:
	for f in $(OBJECTS) .none; do \
		(test $$f = .none || test ! -e "$$f" || rm -f $$f); \
	done
	for f in $(SINGLE_FILE_PROGRAMS) $(TARGETS) .none; do \
		(test $$f = .none || test ! -e "$$f" || rm -f $$f); \
		(test $$f = .none || test ! -e "$$f.o" || rm -f $$f.o); \
		(test $$f = .none || test ! -e "$$f.obj" || rm -f $$f.obj); \
		(test $$f = .none || test ! -e "$$f$(EXEC_SUFFIX)" || rm -f $$f$(EXEC_SUFFIX)); \
	done

.SUFFIXES: .c .cpp .cxx .cc .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

.cxx.o:
	$(CXX) $(CFLAGS) -c $< -o $@

.cc.o:
	$(CXX) $(CFLAGS) -c $< -o $@

