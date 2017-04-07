# XAL_PATH: Paths of the XtratuM distribution
XAL_PATH=../../..

# XMLCF: path to the XML configuration file
XMLCF=../xm_cf.ia32.xml

# PARTITION: partition identifier name
PARTITION=spare-host

all: $(PARTITION).xef
include $(XAL_PATH)/common/rules.mk

SRCS = $(sort $(wildcard src/*.c))
OBJS = $(patsubst %.c, %.o, $(SRCS))

$(if $(findstring undefined,$(origin ID)),\
	$(error "ID is not defined. Please provide partition ID (e.g. make ID=0)"))

$(PARTITION).o: $(OBJS)
	@$(LD) -o $@ $^ $(LDFLAGS) -Ttext=$(call xpathstart,$(shell expr $(ID) + 1),$(XMLCF))
$(PARTITION).xef: $(PARTITION).o
	@$(XEF) $^ -o $@ -i $(ID)
%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $^
#clean:
#	@rm -f $(PARTITION).*
#	@rm -f src/*.o
