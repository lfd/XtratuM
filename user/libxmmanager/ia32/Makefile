TARG=xmmanager.o
all: $(TARG)
include ../../../xmconfig
include $(XTRATUM_PATH)/user/rules.mk

LIBXMMANAGER=${XTRATUM_PATH}/user/libxmmanager
LIB=libxmmanager.a
OBJS=\
	 $(LIBXMMANAGER)/common/xmmanager.o\
	 $(LIBXMMANAGER)/common/hypervisor.o\
	 $(LIBXMMANAGER)/common/comm.o\
	 $(LIBXMMANAGER)/common/trace.o\
	 $(LIBXMMANAGER)/common/hm.o\

$(TARG): ${LIB}
${LIB}: ${OBJS}
	@${AR} cr $@ $^

CFLAGS += -I$(LIBXMMANAGER)/include/ -I$(XTRATUM_PATH)/user/examples/common
LDFLAGS +=  -L$(LIBXM_PATH) --start-group $(LIBGCC) -lxm --end-group

xmmanager: xmmanager.o ${LIB}
	${LD} $^ -o $@ ${LDFLAGS} 
%.o: %.c
	${CC} ${CFLAGS} $^ -c -o $@

clean:
	@${RM} ${INSTALL} ${LIB} ${OBJS} *.o common/*.o
