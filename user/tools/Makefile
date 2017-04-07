all: tools

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

DIRS=\
	xmpack\
	xmcparser\
	rswbuild\
	build_xmc\
	elf2xef\

tools:
	@for dir in $(DIRS) ; do \
		echo "  - tools/$$dir"; \
		$(MAKE) -C $$dir install > /dev/null ; \
	done

clean:
	@find -name "*~" -exec rm '{}' \;
	@find -name "*.o" -exec rm '{}' \;
	@find -name "*.a" -exec rm '{}' \;
	@for dir in $(TOOLS_SUBDIRS) ; do \
		$(MAKE) -C $$dir clean > /dev/null ; \
	done
