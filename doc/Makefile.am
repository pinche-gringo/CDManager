## Process this file with automake to produce Makefile.in

INSTALL_DOCU=CDManager.html.en CDManager.html.es CDManager.html.de
EXTRA_DIST=$(INSTALL_DOCU)

docudir=$(datadir)/doc/@PACKAGE@

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(docudir)
	@list='$(INSTALL_DOCU)'; for f in $$list ; do \
	  echo " $(INSTALL_DATA) $$f $(DESTDIR)$(docudir)/$$f"; \
	  $(INSTALL_DATA) $$f $(DESTDIR)$(docudir)/$$f; \
	 done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	@list='$(INSTALL_DOCU)'; for f in $$list ; do \
	  echo " rm -f $(DESTDIR)$(docudir)/$$f"; \
	  rm -f $(DESTDIR)$(docudir)/$$f; \
	 done
	rmdir -f $(DESTDIR)$(docudir)
