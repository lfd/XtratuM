
INSTALLDIR ?= $(XTRATUM_PATH)/user/bin
INSTALLFILES=$(patsubst %,$(INSTALLDIR)/%,$(INSTALL))

$(INSTALLDIR)/%: %
	@cp $(*) $(INSTALLDIR)/$(*)

install:	$(INSTALL) $(INSTALLFILES)

uninstall:
	@rm -f $(INSTALLFILES)

.PHONY: install uninstall
