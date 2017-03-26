include ../Defs.mak

LDFLAGS += -lfakeps2sdk -L../fake_sdk
LDFLAGS += -liomanX -L../iomanX

CFLAGS += -I. -I./include -I./irx $(CFLAGS2)

BINARY ?= libpfs.a
SOURCES += _init.c
SOURCES += irx/pfs_fio.c irx/pfs_fioctl.c irx/pfs.c
SOURCES += src/bitmap.c src/block.c src/blockWrite.c src/cache.c src/dir.c src/inode.c src/journal.c src/misc.c src/super.c src/superWrite.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(AR) rcs $@ $^
