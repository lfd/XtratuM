all: objs

include ../../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LOBJS := $(OBJS)

-include dep.mk

CC = $(HOSTCC)
CFLAGS = $(HOSTCFLAGS) --include xm_inc/config.h -O2 -Wall -D_GNU_SOURCE `xml2-config --cflags`

objs: $(OBJS)
	@$(HOSTLD) $(HOSTLDFLAGS) -r -o arch.o $(LOBJS)
