# @subdirs@ are the configured subproject directories
SUBDIRS = m4m @subdirs@

EXTRA_DIST = COPYING.LIB README.gphoto-suite

ACLOCAL_AMFLAGS = -I m4m

include $(top_srcdir)/tools/Makefile-files

#######################################################################
# Help for the maintainer
#

.PHONY: update-manpages
update-manpages:
	if test -d "$(top_builddir)/src/gphoto2-manual"; then \
		cp "$(top_builddir)/src/gphoto2-manual/src/man/libgphoto2.3" \
			"$(top_builddir)/src/libgphoto2/doc/"; \
		cp "$(top_builddir)/src/gphoto2-manual/src/man/libgphoto2_port.3" \
			"$(top_builddir)/src/libgphoto2/doc/"; \
		cp "$(top_builddir)/src/gphoto2-manual/src/man/gphoto2.1" \
			"$(top_builddir)/src/gphoto2/doc/"; \
	fi; exit 0

# End of Makefile.am.
