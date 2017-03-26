include ../Defs.mak

LDFLAGS += -lfakeps2sdk -L../fake_sdk

CFLAGS += $(CFLAGS2)

BINARY ?= libiomanX.a
SOURCES += iomanX.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(AR) rcs $@ $^
