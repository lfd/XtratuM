all: build_xmc

INSTALL=build_xmc

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
include $(XTRATUM_PATH)/user/install.mk

build_xmc: build_xmc.in
	# replace by s/.../$(\1)/
	cp $^ $@
	sed -i "s|\$$TARGET_CC |$(TARGET_CC) |g" $@
	sed -i "s|\$$TARGET_CFLAGS |$(TARGET_CFLAGS) |g" $@
	sed -i "s|\$$TARGET_CFLAGS_ARCH |$(TARGET_CFLAGS_ARCH) |g" $@
	sed -i "s|\$$TARGET_LD |$(TARGET_LD) |g" $@
	sed -i "s|\$$TARGET_LDFLAGS_ARCH |$(TARGET_LDFLAGS_ARCH) |g" $@
	sed -i "s|\$$TARGET_AS |$(TARGET_AS) |g" $@
	sed -i "s|\$$TARGET_OBJCOPY |$(TARGET_OBJCOPY) |g" $@

	sed -i "s|\$$XTRATUMVERSION|$(XTRATUMVERSION)|g" $@
