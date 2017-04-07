all: libxm

LIBXMLINUX_PATH=..
include ${LIBXMLINUX_PATH}/common/rules.mk

LIB=libxm-linux.a
OBJS=\
	hypercalls.o\
	hypervisor.o\
	comm.o\
	status.o\
	trace.o\
	hm.o\
	init.o\

%.o: %.c
	@${HOSTCC} ${HOSTCFLAGS} $^ -c -o $@

libxm: ${LIB}
	@cp ${LIB} ${LIBXMLINUX_PATH}
${LIB}: ${OBJS}
	@${AR} cr $@ $^
clean:
	@${RM} ${LIB} ${OBJS}
