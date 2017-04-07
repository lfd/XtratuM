SRCS := $(sort $(wildcard *.c *.S))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS := $(OBJS) $(patsubst %.S,%.o,$(wildcard *.S))

all: objs

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

objs: $(OBJS)
	$(LD) $(LDFLAGS) -r -o kmm.o $(OBJS)
