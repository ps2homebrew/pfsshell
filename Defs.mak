
ifeq ($(MINGW32),1)
CC = i686-w64-mingw32-gcc
AR = i686-w64-mingw32-ar
CFLAGS += -DUSE_LOCAL_TZ -DNEED_GETLINE
else
AR = ar
endif

CFLAGS += -O0 -g -Wall -Wno-unused-value -Wno-format

CFLAGS2 += -I../fake_sdk/include

%.o : %.c
	@echo -e "\tCC  $<"
	@$(CC) -c $(CFLAGS) -o $@ $<
