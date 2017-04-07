# This example assumes that you are using the
# precompiled Linux kernel at linuxloader-server

# XAL_PATH: path to the XTRATUM directory
XAL_PATH=../../..

# XMLCF: path to the XML configuration file
XMLCF=../xm_cf.ia32.xml

# PARTITIONS: partition files (xef format) composing the example
PARTITION=linux-ioserver.xef

# LINUXTOOLS_PATH: path to the LINUX TOOLS
LINUXTOOLS_PATH=$(XTRATUM_PATH)/../linux

all: $(PARTITION)
include $(XAL_PATH)/common/rules.mk

$(if $(findstring undefined,$(origin ID)),\
	$(error "ID is not defined. Please provide partition ID (e.g. make ID=0)"))

linuxloader:
	@make -C $(LINUXTOOLS_PATH)/loader `pwd`/linuxloader

# XEF program gets -i option with the partition identifier
linux-ioserver.xef: linuxloader
	@$(XEF) $< -o $@ -i $(ID)

clean:
	@rm -f linuxloader
	@rm -f linux-ioserver.xef

