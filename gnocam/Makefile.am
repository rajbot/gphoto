if HAVE_GNOCAM
GNOCAM_DIR = idl gnocam bonobo-moniker
endif

if HAVE_CAPPLET
CAPPLET_DIR = capplet
endif

if HAVE_APPLET
APPLET_DIR = applet
endif

if HAVE_VFS_MODULE
VFS_MODULE_DIR = gnome-vfs-module
endif

SUBDIRS =			\
 	art			\
	macros			\
	libgnocam		\
	$(CAPPLET_DIR)		\
	$(APPLET_DIR)		\
	$(VFS_MODULE_DIR)	\
	$(GNOCAM_DIR)		\
	tests			\
	intl			\
	po

EXTRA_DIST =\
 AUTHORS\
 README\
 THANKS\
 TODO\
 autogen.sh\
 xml-i18n-merge.in\
 xml-i18n-update.in\
 xml-i18n-extract.in

