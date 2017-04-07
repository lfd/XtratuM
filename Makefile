.PHONY: xm showsizes

all: xm

include xmconfig

ifndef XTRATUM_PATH
XTRATUM_PATH=.
-include path.mk

path.mk:
	@/bin/echo -e "\n# Automatically added by XM" > path.mk
	@/bin/echo -e "# Please don't modify" >> path.mk
	@/bin/echo -e "export XTRATUM_PATH=`pwd`" >> path.mk
	@/bin/echo -e "`user/bin/scmversion`" >> path.mk
	@cat path.mk >> xmconfig
	@$(RM) -f path.mk
endif

include version
include config.mk

config oldconfig silentoldconfig menuconfig defconfig:
	@exec echo -e "\n> Building Kconfig:";
	@$(MAKE) -s -C scripts/kconfig conf mconf || exit 1 ;
	@exec echo -e "Configure the XtratuM sources:"
	@$(MAKE) -s -C $(XTRATUM_PATH)/core $(MAKECMDGOALS);
	@exec echo -e "Configure the resident software sources:"
	@exec echo -e "[Press enter to enter the configuration utility]"
	@read dummy
	@$(MAKE) -s -C $(XTRATUM_PATH)/user/bootloaders/rsw $(MAKECMDGOALS);
	@exec echo -e "Configure the XAL sources:"
	@exec echo -e "[Press enter to enter the configuration utility]"
	@read dummy
	@$(MAKE) -s -C user/xal $(MAKECMDGOALS)  || exit 1 ;
	@exec echo -e "Next, you may run 'make'"


xm: $(XTRATUM_PATH)/core/include/autoconf.h
	@exec echo -en "\n> Configuring and building the \"XtratuM hypervisor\"";
	@$(MAKE) -s -C core $(MAKECMDGOALS)  || exit 1 ;
	@exec echo -en "\n> Configuring and building the \"User utilities\"";
	@$(MAKE) -s -C user $(MAKECMDGOALS)  || exit 1 ;


distclean: clean
	@exec echo -e "> Cleaning up XM";
	@exec echo -e "  - Removing dep.mk Rules.mk files";
	@find -type f -name "dep.mk" -exec rm '{}' \;
	@find -type f -name "dephost.mk" -exec rm '{}' \;
	@find -type f -name ".config" -exec rm '{}' \;
	@find -type f -name ".config.old" -exec rm '{}' \;
	@find -type f -name "autoconf.h" -exec rm '{}' \;
	@find -type f -name ".menuconfig.log" -exec rm '{}' \;
	@find -type f -name "mconf" -exec rm '{}' \;
	@find -type f -name "conf" -exec rm '{}' \;
	@find -type f -name "partition?" -exec rm '{}' \;
	@find -type f -name "resident_sw" -exec rm '{}' \;
	@find -type f -name "*.xmc.c" -exec rm '{}' \;
	@find -type f -name "xm_cf" -exec rm '{}' \;
	@find -type f -name "rswbuild" -exec rm '{}' \;
	@find -type f -name "build_xmc" -exec rm '{}' \;
	@find -type f -name "bpatch" -exec rm '{}' \;
	@find -type f -name "xmpack" -exec rm '{}' \;
	@find -type f -name "xmcparser" -exec rm '{}' \;
	@find -type l -a -not -name "sh" -exec rm '{}' \;
	@$(RM) -rf $(XTRATUM_PATH)/core/include/config/*
	@$(RM) -rf $(XTRATUM_PATH)/user/bootloaders/rsw/include/config/* $(XTRATUM_PATH)/user/bootloaders/rsw/$(ARCH)/rsw.lds
	@$(RM) -rf $(XTRATUM_PATH)/user/xi/include/config/*	
	@$(RM) -f $(XTRATUM_PATH)/xmconfig $(XTRATUM_PATH)/core/include/autoconf.h $(XTRATUM_PATH)/core/include/$(ARCH)/asm_offsets.h $(XTRATUM_PATH)/core/include/$(ARCH)/brksize.h $(XTRATUM_PATH)/core/include/$(ARCH)/ginfo.h $(XTRATUM_PATH)/scripts/lxdialog/lxdialog $(XTRATUM_PATH)/core/xm_core core/xm_core.bin $(XTRATUM_PATH)/core/kernel/$(ARCH)/xm.lds $(XTRATUM_PATH)/scripts/extractinfo
#	@$(RM) -f $(XTRATUM_PATH)/user/tools/xmcparser/xmc.xsd.in $(XTRATUM_PATH)/user/tools/xmcparser/xmc.xsd
	@$(RM) -f $(XTRATUM_PATH)/core/Kconfig.ver
	@exec echo -e "> Done";

clean:
	@exec echo -e "> Cleaning XM";
	@exec echo -e "  - Removing *.o *.a *~ files";
	@find -name "*~" -exec rm '{}' \;
	@find -name "*.o" -exec rm '{}' \;
	@find -name "*.a" -exec rm '{}' \;
	@find -name "*.gcno" -exec rm '{}' \;
	@find -name "*.bin" -exec rm '{}' \;
	@find -name "dep.mk" -exec rm '{}' \;
	@$(RM) -f $(XTRATUM_PATH)/user/tools/xmcparser/xmc.xsd.in $(XTRATUM_PATH)/user/tools/xmcparser/xmc.xsd
	@exec echo -e "> Done";

DISTRO	= xtratum-$(XTRATUMVERSION)
DISTROTMP=/tmp/$(DISTRO)-$$PPID
DISTROTAR = $(DISTRO).tar.bz2
$(DISTROTAR): # xm
	@$(RM) $(DISTROTAR)
	@#make -s -C user/xal/examples clean
	@mkdir $(DISTROTMP) || exit 0
	@user/bin/xmdistro $(DISTROTMP)/$(DISTRO) $(DISTROTAR)
	@$(RM) -r $(DISTROTMP)

DISTRORUN = $(DISTRO).run
DISTROLABEL= "XtratuM binary distribution $(XTRATUMVERSION): "
$(DISTRORUN): $(DISTROTAR)
	@which makeself >/dev/null || (echo "Error: makeself program not found; install the makeself package" && exit -1)
	@mkdir $(DISTROTMP) || exit 0
	@tar xf $(DISTROTAR) -C $(DISTROTMP)
	@/bin/echo "> Generating self extracting binary distribution \"$(DISTRORUN)\""
	@makeself --bzip2 $(DISTROTMP)/$(DISTRO) $(DISTRORUN) $(DISTROLABEL) ./xtratum-installer > /dev/null 2>&1
	@#| tr "\n" "#" | sed -u "s/[^#]*#/./g; s/..././g"
	@$(RM) $(DISTROTAR)
	@$(RM) -r $(DISTROTMP)
	@/bin/echo -e "> Done\n"

distro-tar: $(DISTROTAR)
distro-run: $(DISTRORUN)

.PHONY: $(DISTROTAR) $(DISTRORUN)
