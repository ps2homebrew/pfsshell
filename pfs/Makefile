include ../Defs.mak

LDFLAGS += -lfakeps2sdk -L../fake_sdk
LDFLAGS += -liomanX -L../iomanX


CFLAGS += $(CFLAGS2)

BINARY ?= libpfs$(SOSUF)
SOURCES += bitmap.c block.c cache.c dir.c inode.c journal.c misc.c \
	   pfs.c pfs_fio.c pfs_fioctl.c super.c _init.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(link_shared)
