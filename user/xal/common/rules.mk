
$(if $(XAL_PATH),, \
	$(warning "The configuration variable XAL_PATH is not set,") \
	$(error "check the \"common/config.mk.dist\" file (see README)."))

ifneq ($(wildcard $(XAL_PATH)/common/config.mk),)
	include $(XAL_PATH)/common/config.mk
	is_installed := 1
else
	ifneq ($(wildcard ../../../../xmconfig),)
		XAL_PATH=../..
		XTRATUM_PATH=../../../..
	endif
	ifneq ($(wildcard ../../../../../xmconfig),)
		XAL_PATH=../../..
		XTRATUM_PATH=../../../../..
	endif
endif

# early detect of misconfiguration and missing variables
$(if $(XTRATUM_PATH),, \
	$(warning "The configuration variable XTRATUM_PATH is not set,") \
	$(error "check the \"common/config.mk.dist\" file (see README)."))

#INCLUDES OF XTRATUM CONFIGURATION
include $(XTRATUM_PATH)/xmconfig
include $(XTRATUM_PATH)/version
ifdef is_installed
include $(XTRATUM_PATH)/user/rules.mk
else
include $(XTRATUM_PATH)/user/rules.mk
endif

#PATHS
ifdef is_installed
XMBIN_PATH=$(XTRATUM_PATH)/bin
XMCORE_PATH=$(XTRATUM_PATH)/user
else
XMBIN_PATH=$(XTRATUM_PATH)/user/bin
XMCORE_PATH=$(XTRATUM_PATH)/core
endif
XALLIB_PATH=$(XAL_PATH)/lib
XALBIN_PATH=$(XAL_PATH)/bin

# APPLICATIONS
XMCPARSER=$(XMBIN_PATH)/xmcparser
XMPACK=$(XMBIN_PATH)/xmpack
RSWBUILD=$(XMBIN_PATH)/rswbuild
XEF=$(XMBIN_PATH)/elf2xef
BUILDXMC=$(XMBIN_PATH)/build_xmc
XPATHSTART=$(XALBIN_PATH)/xpathstart
GRUBISO=$(XALBIN_PATH)/grub_iso

# XM CORE
XMCORE_ELF=$(XMCORE_PATH)/xm_core
XMCORE_BIN=$(XMCORE_PATH)/xm_core.bin
XMCORE=$(XMCORE_PATH)/xm_core.xef

#LIBRARIES
LIB_XM=-lxm
LIB_XAL=-lxal

#FLAGS
CFLAGS += -I$(XAL_PATH)/include -fno-builtin
LDFLAGS += -u start -u __xmImageHdr -T$(XALLIB_PATH)/loader.lds\
	-L$(LIBXM_PATH) -L$(XALLIB_PATH)\
	--start-group $(LIBGCC) $(LIB_XM) $(LIB_XAL) --end-group

# ADDRESS OF EACH PARTITION
# function usage: $(call xpathstart,partitionid,xmlfile)
# xpathstart = $(shell $(XPATHSTART) $(1) $(2))
xpathstart = $(shell $(XAL_PATH)/bin/xpath -c -f $(2) '/xm:SystemDescription/xm:PartitionTable/xm:Partition['$(1)']/xm:PhysicalMemoryAreas/xm:Area[1]/@start')

checkfile = $(findstring $(1),$(wildcard $(1)))

%.xef:  %
	$(XEF) $< -o $@

%.xef.xmc: %.bin.xmc
	$(XEF) $< -o $@

xm_cf.bin.xmc: xm_cf.c.xmc
	$(BUILDXMC) $< $@

xm_cf.c.xmc: $(XMLCF)
	$(XMCPARSER) -s $(XTRATUM_PATH)/user/tools/xmcparser/xmc.xsd -o $@ $^

resident_sw: container.bin
	$(RSWBUILD) $^ $@

resident_sw.iso: resident_sw
	$(GRUBISO) $@ $^

distclean: clean
	@$(RM) *~

clean:
	@$(RM) $(PARTITIONS) $(patsubst %.bin,%, $(PARTITIONS)) $(patsubst %.xef,%, $(PARTITIONS))
	@find -name "*.xef" -exec $(RM) '{}' \;
	@find \( -name '*.a' -o -name '*.o' -o -name '*.xmc' -o -name 'dep.mk' \) -exec $(RM) '{}' \;
	@find \( -name '*.bin' -o -name resident_sw -o -name linuxloader -o -name xm_cf -o -name '*.gz' \) -exec rm '{}' \;

install: resident_sw
	cp $^ /boot/xm/$(shell basename `pwd`).xm
