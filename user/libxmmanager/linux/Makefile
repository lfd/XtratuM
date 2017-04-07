#INSTALLDIR=$(XTRATUM_PATH)/user/linux/bin
INSTALL=xmmanager

all: $(INSTALL)
include ../../../xmconfig
include $(XTRATUM_PATH)/user/install.mk

LIBXMMANAGER=${XTRATUM_PATH}/user/libxmmanager

LIB=libxmmanager.a
OBJS=\
	 $(LIBXMMANAGER)/common/xmmanager.o\
	 $(LIBXMMANAGER)/common/hypervisor.o\
	 $(LIBXMMANAGER)/common/comm.o\
	 $(LIBXMMANAGER)/common/trace.o\
	 $(LIBXMMANAGER)/common/hm.o\

${INSTALL}: ${LIB}
${LIB}: ${OBJS}
	@${AR} cr $@ $^

LIBXMLINUX_PATH=${XTRATUM_PATH}/user/linux/libxm-linux

HOSTCFLAGS += -I$(LIBXMMANAGER)/include/ -I${LIBXMLINUX_PATH}/include/ -I${XTRATUM_PATH}/user/libxm/include -Wall -m32 -static -g
HOSTLDFLAGS += -L${LIBXMLINUX_PATH} -lxm-linux

xmmanager: xmmanager.o ${LIB}
	${HOSTCC} ${HOSTCFLAGS} $^ -o $@ ${HOSTLDFLAGS}
	@install -s $@ $(LIBXMLINUX_PATH)/../bin
%.o: %.c
	${HOSTCC} ${HOSTCFLAGS} $^ -c -o $@

clean:
	@${RM} ${INSTALL} ${LIB} ${OBJS} *.o common/*.o
