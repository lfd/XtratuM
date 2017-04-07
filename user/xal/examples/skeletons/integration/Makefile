# XAL_PATH: Paths of the XtratuM distribution
XAL_PATH=../../..

# XMLCF: path to the XML configuration file
XMLCF=../xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) to integrate
PARTITIONS=linux-ioserver.xef linux-partition.xef xal-partition.xef spare-host.xef

# CONTAINTER: container of the xtratum core, the partitions and the xm_cf configuration
CONTAINER=container.bin

# LINUXTOOLS_PATH: path to the LINUX TOOLS
LINUXTOOLS_PATH = $(XTRATUM_PATH)/../linux/

all: resident_sw
include $(XAL_PATH)/common/rules.mk

INITRDFILES=\
	$(LINUXTOOLS_PATH)/bin/xmmanager\

initrd.img.gz: # $(INITRDFILES)
	@$(LINUXTOOLS_PATH)/bin/mkinitramfs init initrd.img tmproot
	@cp -f $(INITRDFILES) tmproot/bin
	@$(LINUXTOOLS_PATH)/bin/mkinitramfs build initrd.img tmproot

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b linux-ioserver.xef:ioserver-cmdline.txt:initrd.img.gz \
	-b linux-partition.xef:linux-cmdline.txt:initrd.img.gz \
	-b xal-partition.xef:p1.cfg\
	-b spare-host.xef:xm_cf.bin.xmc\

$(CONTAINER): $(PARTITIONS) xm_cf.bin.xmc initrd.img.gz
	$(XMPACK) build $(PACK_ARGS) $@

.PHONY: initrd.img.gz
