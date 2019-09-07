include ../Defs.mak

BINARY ?= libfakeps2sdk.a
SOURCES += atad.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(GEN_STATIC) $@ $^
