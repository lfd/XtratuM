# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition0.xef partition1.xef partition2.xef 

all: container.bin resident_sw
include $(XAL_PATH)/common/rules.mk

partition0.xef: partition.o
	$(TARGET_LD) -o partition0 $^ $(TARGET_LDFLAGS) -Ttext=$(call xpathstart,1,$(XMLCF))
	@$(XEF) partition0 -o $@ -i 0

partition1.xef: partition.o
	$(TARGET_LD) -o partition1 $^ $(TARGET_LDFLAGS) -Ttext=$(call xpathstart,2,$(XMLCF))
	@$(XEF) partition1 -o $@ -i 1

partition2.xef: sys_partition.o
	$(TARGET_LD) -o partition2 $^ $(TARGET_LDFLAGS) -Ttext=$(call xpathstart,3,$(XMLCF))
	@$(XEF) partition2 -o $@ -i 2

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b partition0.xef \
	-b partition1.xef \
	-b partition2.xef

container.bin: $(PARTITIONS) xm_cf.bin.xmc
	$(XMPACK) build $(PACK_ARGS) $@
	
	