all: dirs

include ../../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

DIRS=\
	libxm-linux\

dirs:
	@for dir in $(DIRS) ; do \
		$(MAKE) -s -C $$dir install; \
	done

initramfs.gz:
	@bin/mkinitramfs init initramfs tmproot
	touch tmproot/newfile
	@bin/mkinitramfs build initramfs tmproot

clean:
	@find -name "*~" -exec rm '{}' \;
	@find -name "*.o" -exec rm '{}' \;
	@find -name "*.a" -exec rm '{}' \;
	@for dir in $(DIRS) ; do \
		$(MAKE) -s -C $$dir clean; \
		$(MAKE) -s -C $$dir uninstall; \
	done
	@$(RM) initramfs.gz
