# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition1.xef

all: container.bin resident_sw
include $(XAL_PATH)/common/rules.mk

partition1.xef: hello.o
	$(LD) -o partition1 $^ $(LDFLAGS) -Ttext=$(call xpathstart,1,$(XMLCF))
	@$(XEF) partition1 -o $@ -i 0

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b partition1.xef \

container.bin: $(PARTITIONS) xm_cf.bin.xmc
	$(XMPACK) build $(PACK_ARGS) $@
