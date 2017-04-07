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
	@$(LD) $(LDFLAGS) -r -o kern.o $(LOBJS)

#	$(CC) $(filter-out -pg,$(CFLAGS)) -c gprof.c

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<