all: libxm.a

include ../../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk
-include dep.mk

SRCS := $(sort $(wildcard *.c))
SRCS := $(SRCS) $(sort $(wildcard *.S))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS := $(OBJS) $(patsubst %.S,%.o,$(wildcard *.S))

libxm.a: dep.mk $(OBJS)
	@$(AR) -r libxm.a $(OBJS) 2> /dev/null

clean:
	@rm -f *.o *.a *~ dep.mk ../include/*~ ../include/arch/*~
