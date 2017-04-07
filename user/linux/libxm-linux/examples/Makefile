# LIBXMLINUX_PATH: path to the XtratuM library for Linux
LIBXMLINUX_PATH=../

all: examples

include ${LIBXMLINUX_PATH}/common/rules.mk

EXAMPLES=\
	hello\
	gettime\
	part_manager\

examples: ${EXAMPLES}
clean:
	@${RM} ${EXAMPLES}

