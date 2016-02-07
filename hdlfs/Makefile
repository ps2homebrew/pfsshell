IOP_BIN  = hdlfs.irx
IOP_OBJS = main.o imports.o

IOP_CFLAGS  += -Wall -fno-builtin
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDK)/iop/include -I$(PS2SDK)/common/include

all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2SDK)/Defs.make
include Rules.make
