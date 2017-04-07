all: objs

include ../../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.host.o,$(wildcard *.c))
LOBJS := $(OBJS)

-include dephost.mk

HOST_CFLAGS += `xml2-config --cflags` --include xm_inc/config.h -D_GNU_SOURCE

objs: $(OBJS)
	@$(HOST_LD) $(HOST_LDFLAGS_ARCH) -r -o devices.o $(LOBJS)
