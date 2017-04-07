SRCS := $(sort $(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LOBJS := $(OBJS)

all: objs

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
ifndef CONFIG_SPARE_SCHEDULING
SRCS := $(filter-out spare.c,$(SRCS))
OBJS := $(filter-out spare.o,$(OBJS))
LOBJS := $(filter-out spare.o,$(LOBJS))
endif
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

objs: $(OBJS)
	@$(LD) $(LDFLAGS) -r -o objects.o $(LOBJS)
