
XML_FILES =\
 gnocam-gallery.xml

H_FILES =\
 gnocam-gallery.h

uidir = $(datadir)/gnome/ui
ui_DATA = $(XML_FILES)

EXTRA_DIST = $(XML_FILES) $(H_FILES)

BUILT_SOURCES = $(XML_FILES:.xml=.h)

%.h : %.xml
	bonobo-ui-extract $^ > $@

