INCLUDES = -I$(top_srcdir) $(KONICA_CFLAGS)

if HAVE_KONICA
noinst_PROGRAMS = test-konica
endif

test_konica_SOURCES =			\
	konica.c	konica.h	\
	lowlevel.c	lowlevel.h	\
	main.c				\
	gpfs-konica.c gpfs-konica.h

test_konica_LDADD = $(top_builddir)/libgpfs/libgpfs.la $(KONICA_LIBS)
