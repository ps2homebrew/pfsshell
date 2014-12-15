include ../Defs.mak

LDFLAGS += -lfakeps2sdk -L../fake_sdk

CFLAGS += $(CFLAGS2)

BINARY ?= libiomanX$(SOSUF)
SOURCES += iomanX.c fake_iomanX.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(link_shared)
