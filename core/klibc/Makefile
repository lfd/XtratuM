SRCS = $(sort $(wildcard *.c))
OBJS = $(patsubst %.c,%.o, $(SRCS))

all: objs

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/core/rules.mk
-include dep.mk

objs: $(OBJS)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -r -o libc.o $(OBJS)
