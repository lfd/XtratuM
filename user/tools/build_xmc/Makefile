all: build_xmc

INSTALL=build_xmc

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
include $(XTRATUM_PATH)/user/install.mk

build_xmc: build_xmc.in
	# replace by s/.../$(\1)/
	cp $^ $@
	sed -i "s|\$$CC |$(CC) |g" $@
	sed -i "s|\$$CFLAGS |$(CFLAGS) |g" $@
	sed -i "s|\$$CFLAGS_ARCH |$(CFLAGS_ARCH) |g" $@
	sed -i "s|\$$LD |$(LD) |g" $@
	sed -i "s|\$$LDFLAGS_ARCH |$(LDFLAGS_ARCH) |g" $@
	sed -i "s|\$$AS |$(AS) |g" $@
	sed -i "s|\$$OBJCOPY |$(OBJCOPY) |g" $@

	sed -i "s|\$$XTRATUMVERSION|$(XTRATUMVERSION)|g" $@
