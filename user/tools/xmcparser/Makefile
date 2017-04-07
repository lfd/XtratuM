.PHONY: xmcparser
all: xmcparser

INSTALL=xmcparser

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
include $(XTRATUM_PATH)/user/install.mk

-include dephost.mk

SRCS=process_xml.c xmc.c xmcparser.c xml2c.c devices.c constraints.c hm.c
OBJS := $(patsubst %.c,%.host.o,$(SRCS))

HOST_CFLAGS += $(HOST_CFLAGS_ARCH) `xml2-config --cflags` -D_GNU_SOURCE -D$(ARCH)
HOST_LDFLAGS = `xml2-config --libs`
HOST_ASFLAGS += $(HOST_ASFLAGS_ARCH)

xmcparser: xmc.xsd sdevtab.host.o edevtab.host.o $(OBJS)
	@$(MAKE) -C $(ARCH) all
	@$(MAKE) -C devices all
	@$(HOST_CC) $(HOST_CFLAGS) -o $@ $(OBJS) sdevtab.host.o $(ARCH)/arch.o devices/devices.o edevtab.host.o $(HOST_LDFLAGS)

xmc.xsd.in: xmc.xsd.S
	@$(TARGET_CC) $(TARGET_ASFLAGS) -x assembler-with-cpp -E -P $< -o $@
xmc.xsd: xmc.xsd.in
	@$(SHELL) applyconfig.bash $(XTRATUM_PATH)/core/.config $< $@

clean:
	@$(RM) -f *.o *~ devices/*.o devices/*~ $(ARCH)/*.o $(ARCH)/*~ xmc.xsd xmc.xsd.in xmcparser dephost.mk devices/dep.mk $(ARCH)/dep.mk devices/*.in devices/dephost.mk
