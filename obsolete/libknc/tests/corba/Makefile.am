INCLUDES = -I$(top_srcdir) $(CORBA_CFLAGS)

CORBA_GENERATED = GNOME_C-common.c GNOME_C-stubs.c GNOME_C.h
$(CORBA_GENERATED): $(GNOME_C_IDL)
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(GNOME_C_IDL)

if HAVE_CORBA
noinst_PROGRAMS = test-corba
endif

test_corba_SOURCES = $(CORBA_GENERATED) test-corba.c
test_corba_LDFLAGS = $(CORBA_LIBS)

CLEANFILES =$(CORBA_GENERATED)
