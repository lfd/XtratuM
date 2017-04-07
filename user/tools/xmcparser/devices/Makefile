all: objs

include ../../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LOBJS := $(OBJS)

-include dephost.mk

HOSTCFLAGS += `xml2-config --cflags` --include xm_inc/config.h -D_GNU_SOURCE

%.o: %.c
	@$(HOSTCC) $(HOSTCFLAGS) -o $@ -c $<

objs: $(OBJS)
	@$(HOSTLD) $(HOSTLDFLAGS) -r -o devices.o $(LOBJS)
