.PHONY: menuconfig dochecklxdialog

all:

include ../../xmconfig

HOSTCFLAGS = -Wall -O2 -D$(ARCH) -Wno-unused-value -Wno-unused-but-set-variable

%.o: %.c
	@$(HOSTCC) $(HOSTCFLAGS) -o $@ -c $<

Kconfig := kernel/$(ARCH)/Kconfig

lxdialog := lxdialog/checklist.o lxdialog/util.o lxdialog/inputbox.o
lxdialog += lxdialog/textbox.o lxdialog/yesno.o lxdialog/menubox.o
check-lxdialog := $(XTRATUM_PATH)/scripts/kconfig/lxdialog/check-lxdialog.sh

HOST_EXTRACFLAGS = $(shell $(CONFIG_SHELL) $(check-lxdialog) -ccflags)
HOST_EXTRALIBS = $(shell $(CONFIG_SHELL) $(check-lxdialog) -ldflags $(HOSTCC))

HOSTCFLAGS += $(HOST_EXTRACFLAGS) -DLOCALE

conf-objs := conf.o zconf.tab.o
mconf-objs := mconf.o zconf.tab.o $(lxdialog)

dochecklxdialog:
	@$(CONFIG_SHELL) $(check-lxdialog) -check $(HOSTCC) $(HOST_EXTRACFLAGS) $(HOST_EXTRALIBS)

mconf: dochecklxdialog $(mconf-objs)
	@$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(mconf-objs) $(HOST_EXTRALIBS)

conf: $(conf-objs)
	@$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(conf-objs) $(HOST_EXTRALIBS)

#LKC_GENPARSER := 1

ifdef LKC_GENPARSER

$(obj)/zconf.tab.c: $(src)/zconf.y
$(obj)/lex.zconf.c: $(src)/zconf.l
$(obj)/zconf.hash.c: $(src)/zconf.gperf

%.tab.c: %.y
	bison -l -b $* -p $(notdir $*) $<
	cp $@ $@_shipped

lex.%.c: %.l
	flex -L -P$(notdir $*) -o$@ $<
	cp $@ $@_shipped

%.hash.c: %.gperf
	gperf < $< > $@
	cp $@ $@_shipped

endif

clean:
	@$(RM) -f *.o mconf conf lxdialog/*.o *~ lxdialog/*~
