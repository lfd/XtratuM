all: sphost.o

XAL_PATH=../xal

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XAL_PATH)/common/rules.mk

-include dep.mk

TARGET_CFLAGS+=-I./include

#sphost.o: sphost.c
#	@$(TARGET_CC) $(TARGET_CFLAGS) -r -o $@ $^ 
	
distclean: exclean

clean: exclean

exclean:
	@rm -f *.o *.a *.o *~ include/*~ dep.mk
