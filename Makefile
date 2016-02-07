include ./Defs.mak

LDFLAGS += -Lapa -Lpfs -Lfake_sdk -LiomanX -Wl,-rpath,. 
LDLIBS += -lpfs -lapa -liomanX -lfakeps2sdk -lpthread -lc

SOURCES += startup.c hl.c util.c shell.c
OBJECTS += $(SOURCES:.c=.o)
BINARY ?= pfsshell$(EXESUF)


all: $(BINARY)

clean:
	$(MAKE) -C pfs/ XC=$(XC) clean
	$(MAKE) -C fake_sdk/ XC=$(XC) clean
	$(MAKE) -C apa/ XC=$(XC) clean
	$(MAKE) -C iomanX/ XC=$(XC) clean
	rm -f $(OBJECTS) $(BINARY)
	rm -f libapa$(SOSUF) libfakeps2sdk$(SOSUF) libiomanX$(SOSUF) libpfs$(SOSUF)

hdd.img:
	dd if=/dev/zero of=hdd.img bs=1024 seek=10000000 count=1

libpfs: libiomanX
	$(MAKE) -C pfs/

libfakeps2sdk:
	$(MAKE) -C fake_sdk/

libapa: libiomanX
	$(MAKE) -C apa/

libiomanX: libfakeps2sdk
	$(MAKE) -C iomanX/

$(BINARY): $(OBJECTS) libfakeps2sdk libiomanX libapa libpfs 
	@echo -e "\tLNK $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)
	@ln -sf apa/libapa$(SOSUF) pfs/libpfs$(SOSUF) \
		iomanX/libiomanX$(SOSUF) fake_sdk/libfakeps2sdk$(SOSUF) ./

