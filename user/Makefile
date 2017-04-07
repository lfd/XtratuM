all: links user
include ../xmconfig
include $(XTRATUM_PATH)/config.mk
include $(XTRATUM_PATH)/user/rules.mk

XMUSR_SUBDIRS = libxm tools bootloaders/rsw xal linux libxmmanager/linux

.PHONY: links clean_links

links:
	@if ! test -d $(LIBXM_PATH)/include/arch; then \
		ln -sf $(LIBXM_PATH)/include/$(ARCH) $(LIBXM_PATH)/include/arch; \
	fi
	@if ! test -d $(LIBXM_PATH)/include/xm_inc; then \
		ln -sf $(XM_CORE_PATH)/include $(LIBXM_PATH)/include/xm_inc; \
	fi

clean_links:
	@$(RM) -f $(LIBXM_PATH)/include/arch $(LIBXM_PATH)/include/xm_inc

user:
	@exec echo -e "\n> Building XM user";
	@for dir in $(XMUSR_SUBDIRS) ; do \
		echo "  - $$dir"; \
		$(MAKE) -C $$dir all ; \
	done
	@exec echo "> Done";

distclean: clean_links
	@make -C ../ distclean
clean:
	@exec echo -e "> Cleaning XM user";
	@exec echo -e "  - Removing *.o *.a *~ files";
	@find -name "*~" -exec rm '{}' \;
	@find -name "*.o" -exec rm '{}' \;
	@find -name "*.a" -exec rm '{}' \;
	@exec echo -e "> Done";

