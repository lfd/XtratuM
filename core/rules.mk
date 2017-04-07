
XM_CORE_PATH=$(XTRATUM_PATH)/core

check_gcc = $(shell if $(TARGET_CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

TARGET_CFLAGS = -Wall -D_XM_KERNEL_ -fno-builtin -nostdlib -nostdinc -D$(ARCH) -fno-strict-aliasing 
TARGET_CFLAGS += -I$(XM_CORE_PATH)/include --include config.h --include $(ARCH)/arch_types.h 
TARGET_CFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)
#-fno-stack-protector

TARGET_CFLAGS += $(TARGET_CFLAGS_ARCH)

# disable pointer signedness warnings in gcc 4.0
TARGET_CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
TARGET_CFLAGS += $(call check_gcc,-fno-stack-protector,)

ifndef CONFIG_NO_GCC_OPT
+TARGET_CFLAGS += -O2
endif

TARGET_ASFLAGS = -Wall -D__ASSEMBLY__ -D_XM_KERNEL_ -fno-builtin -D$(ARCH) -nostdlib -nostdinc 
TARGET_ASFLAGS += -I$(XM_CORE_PATH)/include --include config.h
TARGET_ASFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)

TARGET_ASFLAGS += $(TARGET_ASFLAGS_ARCH)

LIBGCC = `$(TARGET_CC) -print-libgcc-file-name $(TARGET_CFLAGS)`

TARGET_LDFLAGS =

TARGET_LDFLAGS += $(TARGET_LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
	TARGET_CFLAGS+=-g
	ASFLAGS+=-g
else
	TARGET_CFLAGS+=-fomit-frame-pointer
endif

%.o: %.c
	$(TARGET_CC) $(TARGET_CFLAGS)  -c $< -o $@

%.o: %.S
	$(TARGET_CC) $(TARGET_ASFLAGS)  -c $< -o $@

dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), clean distclean),)
	@for file in $(SRCS) ; do \
		$(TARGET_CC) $(TARGET_CFLAGS) -M $$file ; \
	done > dep.mk
endif

#distclean:
#	@rm -f *.o *~ dep.mk
