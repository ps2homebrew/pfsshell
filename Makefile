include ./Defs.mak


# ifeq ($(MINGW32),1)
# LDFLAGS += -static
# endif

SOURCES += startup.c hl.c util.c shell.c
OBJECTS += $(SOURCES:.c=.o)
ifeq ($(MINGW32),1)
BINARY ?= pfsshell.exe
else
BINARY ?= pfsshell
endif


all: $(BINARY)

clean:
	$(MAKE) -C pfs clean
	$(MAKE) -C fake_sdk clean
	$(MAKE) -C apa clean
	$(MAKE) -C iomanX clean
	$(MAKE) -C hdlfs clean
	rm -f $(OBJECTS) $(BINARY)

hdd.img:
	dd if=/dev/zero of=$@ bs=1024 seek=10000000 count=1

pfs/libpfs.a: pfs iomanX/libiomanX.a
	$(MAKE) -C $<

fake_sdk/libfakeps2sdk.a: fake_sdk
	$(MAKE) -C $<

apa/libapa.a: apa iomanX/libiomanX.a
	$(MAKE) -C $<

iomanX/libiomanX.a: iomanX fake_sdk/libfakeps2sdk.a
	$(MAKE) -C $<

hdlfs/libhdlfs.a: hdlfs iomanX/libiomanX.a
	$(MAKE) -C $<

$(BINARY): $(OBJECTS) iomanX/libiomanX.a apa/libapa.a pfs/libpfs.a hdlfs/libhdlfs.a fake_sdk/libfakeps2sdk.a
	@echo -e "\tLNK $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LDLIBS) $^
