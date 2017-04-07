
$(if $(LIBXMLINUX_PATH),, \
	$(warning "The configuration variable LIBXMLINUX_PATH is not set,") \
	$(error "check the \"common/config.mk.dist\" file (see README)."))

ifneq ($(LIBXMLINUX_PATH)/common/config.mk, $(wildcard $(LIBXMLINUX_PATH)/common/config.mk))
XTRATM_PATH = ../../../..
else
include $(LIBXMLINUX_PATH)/common/config.mk
is_installed := 1
endif

include ${XTRATUM_PATH}/xmconfig

LIBXM_PATH=$(XTRATUM_PATH)/user/libxm

HOSTCFLAGS += -m32 -static -Wall -D${ARCH} -I${LIBXM_PATH}/include
HOSTCFLAGS += -I${LIBXMLINUX_PATH}/include/ 
HOSTLDFLAGS = -L${LIBXMLINUX_PATH} -lxm-linux

%: %.c
	@${HOSTCC} ${HOSTCFLAGS} $^ -o $@ ${HOSTLDFLAGS}
