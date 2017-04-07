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

HOSTCFLAGS+=$(CFLAGS_ARCH)

%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ -c $<

%.o: %.S
	$(HOSTCC) $(HOSTASFLAGS) -o $@ -c $<

elf2xef: $(OBJS)
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $(OBJS)

clean:
	@$(RM) -f *.o *~ elf2xef $(ARCH)/*~ $(ARCH)/*.o dephost.mk
