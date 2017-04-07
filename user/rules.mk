XM_CORE_PATH=$(XTRATUM_PATH)/core
LIBXM_PATH=$(XTRATUM_PATH)/user/libxm

check_gcc = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

HOST_CFLAGS = -Wall -O2 -D$(ARCH) -I$(LIBXM_PATH)/include -m32
HOST_CFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-uninitialized # TODO
HOST_LDFLAGS =
HOSTASFLAGS = -m32

TARGET_CFLAGS = -Wall -O2 -nostdlib -nostdinc -D$(ARCH) -fno-strict-aliasing -fomit-frame-pointer
TARGET_CFLAGS += -I$(LIBXM_PATH)/include --include xm_inc/config.h --include xm_inc/arch/arch_types.h

TARGET_CFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)

# disable pointer signedness warnings in gcc 4.0
TARGET_CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
TARGET_CFLAGS += $(call check_gcc,-fno-stack-protector,)
TARGET_CFLAGS += $(TARGET_CFLAGS_ARCH)

TARGET_ASFLAGS = -Wall -O2 -D__ASSEMBLY__ -fno-builtin -D$(ARCH) 
TARGET_ASFLAGS += -I$(LIBXM_PATH)/include -nostdlib -nostdinc --include xm_inc/config.h
TARGET_ASFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)
TARGET_ASFLAGS += $(TARGET_ASFLAGS_ARCH)
LIBGCC := `$(TARGET_CC) -print-libgcc-file-name $(TARGET_CFLAGS_ARCH)`

TARGET_LDFLAGS = $(TARGET_LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
	CFLAGS+=-g -D_DEBUG_
else
	CFLAGS+=-fomit-frame-pointer
endif

%.host.o: %.c
	$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

%.host.o: %.S
	$(HOST_CC) $(HOST_ASFLAGS) -o $@ -c $<

%.o: %.c
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

%.o: %.S
	$(TARGET_CC) $(TARGET_ASFLAGS) -o $@ -c $<

.PHONY: $(clean-targets) $(config-targets)
dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(SRCS) ; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -M $$file | sed -e "s/.*:/`dirname $$file`\/&/" ; \
	done > dep.mk
endif

dephost.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(SRCS) ; do \
		$(HOST_CC) $(HOST_CFLAGS) -M $$file | sed -e "s/\(.*\).o:/`dirname $$file`\/\1.host.o:/" ; \
	done > dephost.mk
endif
