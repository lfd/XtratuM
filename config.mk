.DEFAULT: all

$(if $(XTRATUM_PATH),, \
	$(warning "The configuration variable XTRATUM_PATH is not set,") \
	$(error "check the \"xmconfig\" file (see README)."))

$(if $(wildcard $(XTRATUM_PATH)/version),, \
	$(warning "No version file found at $(XTRATUM_PATH),") \
	$(error "see the README in the base directory"))

include $(XTRATUM_PATH)/version

clean-targets := clean distclean depend
config-targets := config xconfig menuconfig oldconfig silentconfig silentoldconfig defconfig
.PHONY: $(clean-targets) $(config-targets)

# skip .config when configuring
ifeq ($(findstring $(MAKECMDGOALS), $(config-targets) $(clean-targets)),)
need_config := 1
endif

# check if the .config exists
ifeq ($(XTRATUM_PATH)/core/.config, $(wildcard $(XTRATUM_PATH)/core/.config))
exists_config := 1
endif

# if there's no .config file warn the user and abort
$(if $(need_config), \
	$(if $(exists_config),, \
	$(warning "No .config file found at $(XTRATUM_PATH)/core,") \
	$(error "run `make menuconfig` in the base directory")))

# if the .config is needed include it
ifdef exists_config
include $(XTRATUM_PATH)/core/.config
# If .config is newer than core/include/autoconf.h, someone tinkered
# with it and forgot to run make oldconfig.

$(XTRATUM_PATH)/core/include/autoconf.h: $(XTRATUM_PATH)/core/.config
	@$(MAKE) -C $(XTRATUM_PATH)/core silentoldconfig MAKEFLAGS=$(patsubst -j%,,$(MAKEFLAGS))

else
# Dummy target needed, because used as prerequisite
$(XTRATUM_PATH)/core/include/autoconf.h: ;
endif

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)
