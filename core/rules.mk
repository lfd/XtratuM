
XM_CORE_PATH=$(XTRATUM_PATH)/core

check_gcc = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

CFLAGS = -Wall -D_XM_KERNEL_ -fno-builtin -nostdlib -nostdinc -D$(ARCH) -fno-strict-aliasing 
CFLAGS += -I$(XM_CORE_PATH)/include --include config.h --include $(ARCH)/arch_types.h 
CFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)
#-fno-stack-protector

CFLAGS += $(CFLAGS_ARCH)

# disable pointer signedness warnings in gcc 4.0
CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
CFLAGS += $(call check_gcc,-fno-stack-protector,)

ifndef CONFIG_NO_GCC_OPT
CFLAGS += -O2
endif

ASFLAGS = -Wall -D__ASSEMBLY__ -D_XM_KERNEL_ -fno-builtin -D$(ARCH) -nostdlib -nostdinc 
ASFLAGS += -I$(XM_CORE_PATH)/include --include config.h
ASFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)

ASFLAGS += $(ASFLAGS_ARCH)

LIBGCC = `$(CC) -print-libgcc-file-name $(CFLAGS)`

LDFLAGS =

LDFLAGS += $(LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
	CFLAGS+=-g
else
	CFLAGS+=-fomit-frame-pointer
endif

dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), clean distclean),)
	@for file in $(SRCS) ; do \
		$(CC) $(CFLAGS) -M $$file ; \
	done > dep.mk
endif

#distclean:
#	@rm -f *.o *~ dep.mk

