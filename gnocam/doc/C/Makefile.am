gnocam_helpdir = $(datadir)/gnome/help/$(PACKAGE)/C

EXTRA_DIST = \
	gnocam-manual.sgml

all: gnocam-manual

gnocam-manual: gnocam-manual.sgml
	-db2html gnocam-manual.sgml

dist-hook:
	mkdir $(distdir)/gnocam-manual
	-cp gnocam-manual/*.html gnocam-manual/*.css $(distdir)/gnocam-manual
	mkdir $(distdir)/gnocam-manual/stylesheet-images
	-cp gnocam-manual/stylesheet-images/* $(distdir)/gnocam-manual/stylesheet-images

install-data-local: gnocam-manual
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(gnocam_helpdir)
	-for file in $(srcdir)/gnocam-manual/*.html $(srcdir)/gnocam-manual/*.css; do \
		basefile=`basename $$file`; \
		$(INSTALL_DATA) $$file $(DESTDIR)$(gnocam_helpdir)/$$basefile; \
	done
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(gnocam_helpdir)/stylesheet-images
	for file in $(srcdir)/gnocam-manual/stylesheet-images/*; do \
		basefile=`basename $$file`; \
		$(INSTALL_DATA) $$file $(DESTDIR)$(gnocam_helpdir)/stylesheet-images/$$basefile; \
	done

gnocam-manual.ps: gnocam-manual.sgml
	-db2ps $<

gnocam-manual.rtf: gnocam-manual.sgml
	-db2rtf $<
