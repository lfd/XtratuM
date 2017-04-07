all: rsw_objs.o

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
#-include dep.mk

RSW_PATH=$(XTRATUM_PATH)/user/bootloaders/rsw

RSW-common = common/rsw.c common/stdc.c

RSW-ia32-SRC=ia32/boot.S

RESIDENT_SW_SRC= $(RSW-common) $(RSW-$(ARCH)-SRC)

RESIDENT_SW := $(patsubst %.S,%.o, $(patsubst %.c,%.o,$(RESIDENT_SW_SRC)))

SRCS=$(RESIDENT_SW_SRC)

CFLAGS+=-I$(RSW_PATH)/include --include config.h -DRSW
ASFLAGS+=-I$(RSW_PATH)/include --include config.h

KCONFIG_AUTOHEADER=$(RSW_PATH)/include/autoconf.h
$(KCONFIG_AUTOHEADER): silentoldconfig

$(ARCH)/rsw.lds: $(KCONFIG_AUTOHEADER) $(ARCH)/rsw.lds.in
	@$(CC) $(ASFLAGS) -x assembler-with-cpp -E -P $(ARCH)/rsw.lds.in -o $(ARCH)/rsw.lds

KCONFIG=$(RSW_PATH)/$(ARCH)/Kconfig
DCONFIG=$(RSW_PATH)/$(ARCH)/defconfig

config:
	@$(XTRATUM_PATH)/scripts/kconfig/conf $(KCONFIG)

oldconfig:
	@$(XTRATUM_PATH)/scripts/kconfig/conf -o $(KCONFIG)

silentoldconfig:
	@mkdir -p $(RSW_PATH)/include/config
	@$(XTRATUM_PATH)/scripts/kconfig/conf -s $(KCONFIG)

menuconfig:
	@$(XTRATUM_PATH)/scripts/kconfig/mconf $(KCONFIG)

defconfig:
	$(XTRATUM_PATH)/scripts/kconfig/conf -d -D $(DCONFIG) $(KCONFIG)

rsw_objs.o: $(ARCH)/rsw.lds $(RESIDENT_SW)
	@$(LD) $(LDFLAGS_ARCH) -r -o rsw_objs.o $(RESIDENT_SW)

clean:
	@$(RM) *.o *~ $(ARCH)/*.o $(ARCH)/*~ $(ARCH)/rsw.lds common/*.o common/*~
