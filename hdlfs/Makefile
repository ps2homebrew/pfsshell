include ../Defs.mak

CFLAGS += -I. $(CFLAGS2) 

BINARY ?= libhdlfs.a
SOURCES += _init.c main.c 
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

$(BINARY): $(OBJECTS)
	$(GEN_STATIC) $@ $^
