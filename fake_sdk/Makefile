include ../Defs.mak

BINARY ?= libfakeps2sdk$(SOSUF)
SOURCES += thsemap.c atad.c \
	    dbg.c err.c
OBJECTS += $(SOURCES:.c=.o)

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

sysclib.o dbg.c err.o:
	@echo -e "\tCC  $(@:.o=.c)"
	@$(CC) $(CFLAGS) $(CFLAGS2) -c -o $@ $(@:.o=.c)

thsemap.o: thsemap.c
	@echo -e "\tCC  $<"
	@$(CC) $(CFLAGS) $(CFLAGS2) -I../$(HDLD) -c -o $@ $<

# override includes
atad.o:
	@echo -e "\tCC  $(@:.o=.c)"
	@$(CC) $(CFLAGS) -I../$(HDLD) -c -o $@ $(@:.o=.c)

$(BINARY): $(OBJECTS)
	$(link_shared)
