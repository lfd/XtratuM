all: libxm.a

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

-include dep.mk

SRCS = $(sort $(wildcard common/*.c)) $(sort $(wildcard $(ARCH)/*.c)) $(sort $(wildcard $(ARCH)/*.S))
OBJS = $(patsubst %.c,%.o, $(sort $(wildcard common/*.c))) $(patsubst %.c,%.o, $(sort $(wildcard $(ARCH)/*.c))) $(patsubst %.S,%.o, $(sort $(wildcard $(ARCH)/*.S)))

libxm.a: $(OBJS)
	@$(TARGET_AR) -c -r libxm.a $(OBJS)

distclean: clean

clean:
	@rm -f *.o *.a *.o *~ include/*~ include/arch/*~ $(ARCH)/*~ $(ARCH)/*.o common/*.o common/*~ dep.mk
