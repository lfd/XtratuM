.PHONY: xmcparser
all: xmcparser

INSTALL=xmcparser

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
include $(XTRATUM_PATH)/user/install.mk

-include dephost.mk

SRCS=process_xml.c xmc.c xmcparser.c xml2c.c devices.c constraints.c hm.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

HOSTCFLAGS += `xml2-config --cflags` -D_GNU_SOURCE -D$(ARCH)

%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ -c $<

%.o: %.S
	$(HOSTCC) $(HOSTASFLAGS) -o $@ -c $<

xmcparser: xmc.xsd sdevtab.o edevtab.o $(OBJS)
	@$(MAKE) -C $(ARCH) all
	@$(MAKE) -C devices all
	@$(HOSTCC) $(HOSTCFLAGS) -o $@ $(OBJS) sdevtab.o $(ARCH)/arch.o devices/devices.o edevtab.o `xml2-config --libs`

xmc.xsd.in: xmc.xsd.S
	@$(CC) $(ASFLAGS) -x assembler-with-cpp -E -P $< -o $@
xmc.xsd: xmc.xsd.in
	@$(SHELL) applyconfig.bash $(XTRATUM_PATH)/core/.config $< $@

clean:
	@$(RM) -f *.o *~ devices/*.o devices/*~ $(ARCH)/*.o $(ARCH)/*~ xmc.xsd xmc.xsd.in xmcparser dephost.mk devices/dep.mk $(ARCH)/dep.mk devices/*.in devices/dephost.mk
