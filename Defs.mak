
ifeq ($(shell uname -s),Darwin)
DARWIN = 1
else
DARWIN = 0
endif

ifeq ($(MINGW32),1)
CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
GEN_STATIC = $(AR) rcs
CFLAGS += -DUSE_LOCAL_TZ -DNEED_GETLINE
else
AR = ar
LIBTOOL = libtool
GEN_STATIC = $(AR) rcs
ifeq ($(DARWIN),1)
GEN_STATIC = $(LIBTOOL) -static -o
endif
endif

CFLAGS += -O0 -g -Wall -Wno-unused-value -Wno-format

CFLAGS2 += -I../fake_sdk/include

%.o : %.c
	@echo -e "\tCC  $<"
	@$(CC) -c $(CFLAGS) -o $@ $<
