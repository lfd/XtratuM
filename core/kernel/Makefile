SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LOBJS := $(OBJS)
#OBJS := $(filter-out gprof.o,$(OBJS))

all: objs

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

objs: $(OBJS)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -r -o kern.o $(LOBJS)

#	$(TARGET_CC) $(filter-out -pg,$(TARGET_CFLAGS)) -c gprof.c

.c.s:
	$(TARGET_CC) $(TARGET_CFLAGS) -S -o $*.s $<