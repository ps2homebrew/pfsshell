include ../Defs.mak

BINARY ?= libfakeps2sdk.a
SOURCES += thsemap.c atad.c dbg.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

thsemap.o: thsemap.c
	@echo -e "\tCC  $<"
	@$(CC) $(CFLAGS) $(CFLAGS2) -c -o $@ $<

$(BINARY): $(OBJECTS)
	ar rcs $@ $^
