all: objs

include ../../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.host.o,${SRCS})

-include dephost.mk

#HOST_CFLAGS += --include xm_inc/config.h  
HOST_CFLAGS += $(HOST_CFLAGS_ARCH) `xml2-config --cflags` -D_GNU_SOURCE -D$(ARCH) -O2 -Wall

objs: $(OBJS)
	@$(HOST_LD) $(HOST_LDFLAGS_ARCH) -r -o arch.o $(OBJS)

clean:
	@$(RM) -f *.o
