INCLUDES = 					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-DLOCALEDIR=\"$(datadir)/locale\"

noinst_PROGRAMS = test-fs
test_fs_LDADD = ../libgpfs/libgpfs.la $(INTLLIBS)
