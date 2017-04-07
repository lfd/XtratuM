# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../..

# XMLCF: path to the XML configuration file
XMLCF=xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITIONS=partition1.xef partition2.xef partition3.xef

all: container.bin resident_sw
include $(XAL_PATH)/common/rules.mk

partition1.xef: hm-monitor.o
	$(LD) -o partition1 $^ $(LDFLAGS) -Ttext=$(call xpathstart,1,$(XMLCF))
	@$(XEF) partition1 -o $@ -i 0

partition2.xef: trace.o
	$(LD) -o partition2 $^ $(LDFLAGS) -Ttext=$(call xpathstart,2,$(XMLCF))
	@$(XEF) partition2 -o $@ -i 1

partition3.xef: trace.o
	$(LD) -o partition3 $^ $(LDFLAGS) -Ttext=$(call xpathstart,3,$(XMLCF))
	@$(XEF) partition3 -o $@ -i 2

PACK_ARGS=-h $(XMCORE_BIN):xm_cf.bin.xmc \
	-b partition1.xef:p1.cfg \
	-b partition2.xef:p1.cfg \
	-b partition3.xef:p1.cfg \

container.bin: $(PARTITIONS) xm_cf.bin.xmc
	@touch p1.cfg
	$(XMPACK) build $(PACK_ARGS) $@
	@rm p1.cfg
