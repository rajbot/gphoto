## Process this file with automake to generate a GNU Makefile

CFLAGS = -O2 -Wall

SUBDIRS = canon \
	casio \
	directory \
	fuji \
	gallery \
	kodak \
	kodak_generic \
	konica \
	konica_qmxxx \
	man \
	minolta \
	mustek \
	nikon \
	philips \
	photopc \
	ricoh \
	samsung \
	sony \
	src

gphotodocdir = $(datadir)/gphoto/doc

gphotodoc_DATA = AUTHORS MANUAL

EXTRA_DIST = $(gphotodoc_DATA) \
	CONTACTS \
	COPYING \
	ChangeLog \
	FAQ \
	INSTALL \
	NEWS \
	NEWS_0_4_0 \
	NEWS_0_4_1 \
	NEWS_0_4_2 \
	PROGRAMMERS \
	README \
	THANKS \
	THEMES \
	TODO \
	configure \
	gphoto.desktop \
	gphoto.spec 

sysdir = $(datadir)/gnome/apps/Graphics
sys_DATA = gphoto.desktop
