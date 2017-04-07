XM_CORE_PATH=$(XTRATUM_PATH)/core
LIBXM_PATH=$(XTRATUM_PATH)/user/libxm

check_gcc = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi)

HOSTCFLAGS = -Wall -O2 -D$(ARCH) -I$(LIBXM_PATH)/include -m32
HOSTCFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-uninitialized # TODO
HOSTLDFLAGS = -m elf_i386
HOSTASFLAGS = -m32

CFLAGS = -Wall -O2 -nostdlib -nostdinc -D$(ARCH) -fno-strict-aliasing -fomit-frame-pointer
CFLAGS += -I$(LIBXM_PATH)/include --include xm_inc/config.h --include xm_inc/arch/arch_types.h

CFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)

# disable pointer signedness warnings in gcc 4.0
CFLAGS += $(call check_gcc,-Wno-pointer-sign,)
# disable stack protector in gcc 4.1
CFLAGS += $(call check_gcc,-fno-stack-protector,)
CFLAGS += $(CFLAGS_ARCH)

ASFLAGS = -Wall -O2 -D__ASSEMBLY__ -fno-builtin -D$(ARCH) 
ASFLAGS += -I$(LIBXM_PATH)/include -nostdlib -nostdinc --include xm_inc/config.h
ASFLAGS += -DCONFIG_VERSION=$(XM_VERSION) -DCONFIG_SUBVERSION=$(XM_SUBVERSION) -DCONFIG_REVISION=$(XM_REVISION)
ASFLAGS += $(ASFLAGS_ARCH)
LIBGCC := `$(CC) -print-libgcc-file-name $(CFLAGS_ARCH)`

LDFLAGS = $(LDFLAGS_ARCH)

ifdef CONFIG_DEBUG
	CFLAGS+=-g -D_DEBUG_
else
	CFLAGS+=-fomit-frame-pointer
endif

.PHONY: $(clean-targets) $(config-targets)
dep.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(SRCS) ; do \
		$(CC) $(CFLAGS) -M $$file | sed -e "s/.*:/`dirname $$file`\/&/" ; \
	done > dep.mk
endif

dephost.mk: $(SRCS)
# don't generate deps  when cleaning
ifeq ($(findstring $(MAKECMDGOALS), $(clean-targets) $(config-targets) ),)
	@for file in $(SRCS) ; do \
		$(HOSTCC) $(HOSTCFLAGS) -M $$file | sed -e "s/.*:/`dirname $$file`\/&/" ; \
	done > dephost.mk
endif
