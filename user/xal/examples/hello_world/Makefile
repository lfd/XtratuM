# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition.xef

all: container.bin resident_sw
include $(XAL_PATH)/common/rules.mk

partition.xef: partition.o
	$(TARGET_LD) -o partition $^ $(TARGET_LDFLAGS) -Ttext=$(call xpathstart,1,$(XMLCF))
	@$(XEF) partition -o $@ -i 0

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b partition.xef \

container.bin: $(PARTITIONS) xm_cf.bin.xmc
	$(XMPACK) build $(PACK_ARGS) $@
	
