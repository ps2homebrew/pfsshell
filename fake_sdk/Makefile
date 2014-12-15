include ../Defs.mak

BINARY ?= libfakeps2sdk$(SOSUF)
SOURCES += thsemap.c sysmem.c atad.c intrman.c sysclib.c \
	   cdvdman.c dev9.c dbg.c err.c
OBJECTS += $(SOURCES:.c=.o)

# include hdl_dump's hio
OBJECTS += hio_probe.o hio_win32.o dict.o osal_unix.o

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)

sysmem.o intrman.o sysclib.o cdvdman.o dev9.o dbg.c err.o:
	@echo -e "\tCC  $(@:.o=.c)"
	@$(CC) $(CFLAGS) $(CFLAGS2) -c -o $@ $(@:.o=.c)

thsemap.o: thsemap.c
	@echo -e "\tCC  $<"
	@$(CC) $(CFLAGS) $(CFLAGS2) -I../$(HDLD) -c -o $@ $<

# override includes
atad.o fake_win32.o:
	@echo -e "\tCC  $(@:.o=.c)"
	@$(CC) $(CFLAGS) -I../$(HDLD) -c -o $@ $(@:.o=.c)

# override includes and more
hio_probe.o hio_win32.o osal_unix.o dict.o osal_win32.o:
	@echo -e "\tCC  $(@:.o=.c)"
	@$(CC) $(CFLAGS) -c -o $@ ../$(HDLD)/$(@:.o=.c)

$(BINARY): $(OBJECTS)
	$(link_shared)
