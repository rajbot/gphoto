
XML_FILES =\
 gnocam-gallery.xml\
 gnocam-main.xml\
 gnocam-preview.xml

H_FILES =\
 gnocam-gallery.h\
 gnocam-main.h\
 gnocam-preview.h

uidir = $(datadir)/gnome/ui
ui_DATA = $(XML_FILES)

EXTRA_DIST = $(XML_FILES) $(H_FILES)

BUILT_SOURCES = $(XML_FILES:.xml=.h)

%.h : %.xml
	bonobo-ui-extract $^ > $@

