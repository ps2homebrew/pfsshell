# -*-makefile-*-

PS2SDK = ../ps2sdk
HDLD = ./hdl_dump

DEBUG = yes

CFLAGS += -D_BUILD_UNIX -m32
CXXFLAGS += -D_BUILD_UNIX -m32
EXESUF =
SOSUF = .dylib

CFLAGS += -O0 -g -D_DEBUG
#no debug: define NDEBUG

CFLAGS += -Wall
CFLAGS2 += -I$(PS2SDK)/common/include
CFLAGS2 += -I$(PS2SDK)/iop/kernel/include
CFLAGS2 += -I$(PS2SDK)/iop/dev9/atad/include

%.o : %.c
	@echo -e "\tCC  $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

define link_shared
	@echo -e "\tLNK $@"
	@$(CC) $(CFLAGS) -shared -o $@ $(OBJECTS) $(LDFLAGS)
endef
