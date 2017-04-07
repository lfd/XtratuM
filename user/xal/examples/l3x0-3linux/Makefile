# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition1.xef partition2.xef partition3.xef 

# LINUX_PATH: path to the Linux Kernel souces used for Linux partitions
LINUX_PATH=

# LINUXTOOLS_PATH: path to the LINUX TOOLS
LINUXTOOLS_PATH = $(XTRATUM_PATH)/user/linux/

all: container.bin resident_sw
include $(XAL_PATH)/common/rules.mk

linuxloader:
	@make -C $(LINUXTOOLS_PATH)/loader `pwd`/linuxloader

partition1.xef: linuxloader
	@$(XEF) linuxloader -o $@ -i 0

partition2.xef: linuxloader
	@$(XEF) linuxloader -o $@ -i 1

partition3.xef: linuxloader
	@$(XEF) linuxloader -o $@ -i 2

INITRDFILES=\
	$(LINUXTOOLS_PATH)/bin/xmmanager\
	$(call checkfile,$(LINUX_PATH)/arch/x86/xm/usr/xmioctl)\

initrd.img.gz: # $(INITRDFILES)
	@$(LINUXTOOLS_PATH)/bin/mkinitramfs init initrd.img tmproot
	@cp -r $(INITRDFILES) tmproot/bin
	@$(LINUXTOOLS_PATH)/bin/mkinitramfs build initrd.img tmproot

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b partition1.xef:cmdline1.txt:initrd.img.gz \
	-b partition2.xef:cmdline2.txt:initrd.img.gz \
	-b partition3.xef:cmdline2.txt:initrd.img.gz

container.bin: $(PARTITIONS) xm_cf.bin.xmc initrd.img.gz
	$(XMPACK) build $(PACK_ARGS) $@

.PHONY: initrd.img.gz
