.PHONY: elf2xef
all: elf2xef

INSTALL=elf2xef

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
include $(XTRATUM_PATH)/user/install.mk

-include dephost.mk

SRCS=main.c $(ARCH)/elf.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

HOST_CFLAGS+=$(CFLAGS_ARCH)

%.o: %.c
	$(HOST_CC) $(HOST_CFLAGS) -o $@ -c $<

%.o: %.S
	$(HOST_CC) $(HOSTASFLAGS) -o $@ -c $<

elf2xef: $(OBJS)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $(OBJS)

clean:
	@$(RM) -f *.o *~ elf2xef $(ARCH)/*~ $(ARCH)/*.o dephost.mk
