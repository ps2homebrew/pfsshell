include ../Defs.mak

LDFLAGS += -L../fake_sdk -lfakeps2sdk 
LDFLAGS += -L../iomanX -liomanX 


CFLAGS += $(CFLAGS2)

BINARY ?= libapa$(SOSUF)
SOURCES += apa.c cache.c hdd.c hdd_fio.c journal.c misc.c _init.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(link_shared)
