# @subdirs@ are the configured subproject directories
SUBDIRS = m4m po i18n-test @subdirs@

EXTRA_DIST = autogen.sh checkout-config.sh checkout-config.txt

ACLOCAL_AMFLAGS = -I m4m

#######################################################################
# Help for the maintainer
#

# Simulate something like
#   EXTRA_DIST_IF_EXIST = ChangeLog.cvs
# If present, ship ChangeLog.cvs in source tarball.
# If not present, don't ship it.
dist-hook:
	if test -f $(srcdir)/ChangeLog.cvs; then \
		cp -p $(srcdir)/ChangeLog.cvs $(distdir)/ChangeLog.cvs; \
	fi

.PHONY: cvs-changelog
cvs-changelog: $(srcdir)/ChangeLog.cvs
	@for subdir in @subdirs@; do (cd "$$subdir" && $(MAKE) $@); done

.PHONY: $(srcdir)/ChangeLog.cvs
$(srcdir)/ChangeLog.cvs:
	if test -f "$(srcdir)/cvs2cl.usermap"; then \
		usermap="--usermap ./cvs2cl.usermap"; \
	else \
		usermap=""; \
	fi; \
	(cd "$(srcdir)" && cvs2cl $${usermap} -f "ChangeLog.cvs")

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

.PHONY: cvs-tag-release
cvs-tag-release:
	@tag="$$(echo "$(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-release" | sed 's|\.|_|g')"; \
	echo "Are you sure you want to CVS tag \`$${tag}' your source files?"; \
	echo "Press Ctrl-C to abort, Enter to continue."; \
	read; \
	cd "$(srcdir)" && cvs tag "$${tag}"

# End of Makefile.am.
