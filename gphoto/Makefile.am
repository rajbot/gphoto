## Process this file with automake to generate a GNU Makefile

SUBDIRS = intl po libgpio barbie canon casio digita dimera directory fuji gallery jd11 kodak \
kodak_generic konica konica_qmxxx man minolta mustek nikon \
philips photopc polaroid ricoh samsung sony src

CFLAGS= @CFLAGS@ -I../libgpio -I../../libgpio -I../../../libgpio 
LDFLAGS= @LDFLAGS@ -L../libgpio -L./libgpio -lgpio

gphotodocdir = $(datadir)/gphoto/doc

gphotodoc_DATA = AUTHORS MANUAL

EXTRA_DIST = $(gphotodoc_DATA) NEWS README THEMES TODO COPYING THANKS \
	ChangeLog FAQ INSTALL PROGRAMMERS gphoto.spec configure \
	gphoto.desktop

sysdir = $(datadir)/gnome/apps/Graphics
sys_DATA = gphoto.desktop
