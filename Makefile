include ./Defs.mak

LDLIBS += -lpthread -lc

SOURCES += startup.c hl.c util.c shell.c
OBJECTS += $(SOURCES:.c=.o)
BINARY ?= pfsshell

all: $(BINARY)

clean:
	$(MAKE) -C pfs/ clean
	$(MAKE) -C fake_sdk/ clean
	$(MAKE) -C apa/ clean
	$(MAKE) -C iomanX/ clean
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

$(BINARY): $(OBJECTS) fake_sdk/libfakeps2sdk.a iomanX/libiomanX.a apa/libapa.a pfs/libpfs.a
	@echo -e "\tLNK $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
