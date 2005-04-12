# The modules

mod_LTLIBRARIES = mod_a.la mod_b.la mod_c.la mod_d.la mod_e.la

# symbol names according to libtool.info
# export all symbols
mod_a_la_SOURCES = dlt-mod.c dlt-mod.h
mod_a_la_LDFLAGS = -module -no-undefined -avoid-version
mod_a_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS) \
	-DMODULE=mod_a

# symbol names according to libtool.info
# export only public symbols according to libtool.info
mod_b_la_SOURCES = dlt-mod.c dlt-mod.h
mod_b_la_LDFLAGS = -module -no-undefined -avoid-version \
	-export-dynamic -export-symbols-regex '^mod_b_LTX_'
mod_b_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS) \
	-DMODULE=mod_b

# straightforward symbol names
# export all symbols
mod_c_la_SOURCES = dlt-mod.c dlt-mod.h
mod_c_la_LDFLAGS = -module -no-undefined -avoid-version \
	-export-dynamic
mod_c_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS)

# straightforward symbol names
# export only public symbols according to explicit list
mod_d_la_DEPENDENCIES = dlt-mod.sym
mod_d_la_SOURCES = dlt-mod.c dlt-mod.h
mod_d_la_LDFLAGS = -module -no-undefined -avoid-version \
	-export-dynamic -export-symbols $(srcdir)/dlt-mod.sym
mod_d_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS)

# straightforward symbol names
# export no symbols
mod_e_la_DEPENDENCIES = dlt-mod-empty.sym
mod_e_la_SOURCES = dlt-mod.c dlt-mod.h
mod_e_la_LDFLAGS = -module -no-undefined -avoid-version \
	-export-dynamic -export-symbols $(srcdir)/dlt-mod-empty.sym
mod_e_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS)


# End of Makefile.am.
