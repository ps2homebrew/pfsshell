include ../Defs.mak

LDFLAGS += -L../fake_sdk -lfakeps2sdk 
LDFLAGS += -L../iomanX -liomanX 


CFLAGS += -I./irx -I./include -I. $(CFLAGS2) 

BINARY ?= libapa.a
SOURCES += _init.c
SOURCES += irx/hdd_fio.c irx/hdd.c
SOURCES += src/apa.c src/cache.c src/journal.c src/misc.c src/password.c src/free.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(AR) rcs $@ $^
