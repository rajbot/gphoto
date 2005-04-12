SUBDIRS = m4m @subdirs@ lib mod bin

EXTRA_DIST = run-test.sh

TESTS = run-test.sh

ACLOCAL_AMFLAGS = -I m4m

# Help the maintainer test and publish
upload.stamp: dist-bzip2
	chmod 0644 $(distdir).tar.bz2
	rsync -avz --bwlimit=14 --progress \
		$(distdir).tar.bz2 \
		sfcfs:batches/queued/
	rsync -avz --bwlimit=14 --progress \
		$(distdir).tar.bz2 \
		h:es/
	true > $@

public: upload.stamp
	@echo "You may want to run"
	@echo "    (cd $(srcdir) && cvs tag $(PACKAGE_TARNAME)-$$(echo "$(PACKAGE_VERSION)" | sed 'y|.-|__|')-release)"

CLEANFILES = upload.stamp
