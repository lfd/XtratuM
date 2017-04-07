all: libs
install: all
TARG=lib/liblinuxloader.a
OBJS=\
	$(patsubst %.S,%.o, $(SRCS))\
	$(patsubst %.c,%.o, $(SRCS))\

SRCS=\
	common/main.c\
	common/stdc.c\
	\
	ia32/boot.S\
	ia32/vmmap.c\
	ia32/linux_boot.c\
	ia32/arch.c\

libs: ${TARG}

ifndef LINUXLOADER_SHIPPED
ifneq (,$(findstring linuxloader,${MAKECMDGOALS}))
$(if $(LINUX_PATH),, $(warning "LINUX_PATH variable is not defined") $(error "Please, define LINUX_PATH"))
endif
endif
$(if $(XTRATUM_PATH),, $(warning "XTRATUM_PATH variable is not defined") $(error "Please, define XTRATUM_PATH"))

include $(XTRATUM_PATH)/xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

$(TARG): $(OBJS)
	@$(AR) cr $@ $^
	@cp $(ARCH)/loader.lds lib/

ASFLAGS+= -I include
CFLAGS += -I include -I /usr/include -DLINUXLOADER

ELF2XEF=$(XTRATUM_PATH)/user/bin/elf2xef
ifeq (,$(findstring $(ELF2XEF),$(wildcard $(ELF2XEF))))
ELF2XEF=$(XTRATUM_PATH)/bin/elf2xef
endif

exclude_sections = $(shell readelf -S $(LINUX_PATH)/vmlinux | awk '/__kcrctab.*[ ]*PROGBITS/ {gsub(/[\[\]]/, "", $$1); print "-x", $$1+1;}')

LOADERTMP_SRC := $(shell mktemp /tmp/loadertmp_XXX.c)
LOADERTMP_OBJ := $(patsubst %.c,%.o, $(LOADERTMP_SRC))
VMLINUX_XEF := $(shell mktemp /tmp/vmlinux_XXX.xef)

%linuxloader:
#	[ -f $(LINUX_PATH)/vmlinux ] && exec echo "LINUX_PATH must point out to a valid container";
ifdef LINUXLOADER_SHIPPED
	@cp -f linuxloader.shipped $@
	@exit 0
else
	@$(ELF2XEF) $(call exclude_sections) -o ${VMLINUX_XEF} $(LINUX_PATH)/vmlinux
	@$(CC) $(CFLAGS_ARCH) -c ${LOADERTMP_SRC} -o ${LOADERTMP_OBJ}
	@$(OBJCOPY) --add-section=.linux=${VMLINUX_XEF} --set-section-flags=.linux=load,alloc ${LOADERTMP_OBJ}
	@$(LD) -q $(LDFLAGS_ARCH) -o $@ ${LOADERTMP_OBJ} -Llib/ -Tlib/loader.lds -llinuxloader -L$(LIBXM_PATH) --start-group -lxm $(LIBGCC) --end-group
	@$(RM) ${LOADERTMP_SRC} ${LOADERTMP_OBJ} ${VMLINUX_XEF}
endif

clean:
	@$(RM) *.o *~ $(ARCH)/*.o common/*.o
distclean: clean
	@$(RM) lib/*

.PHONY: clean linuxloader
