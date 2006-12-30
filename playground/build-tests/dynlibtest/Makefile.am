SUBDIRS = m4m @subdirs@ lib mod bin

EXTRA_DIST = run-test.sh

TESTS = run-test.sh

ACLOCAL_AMFLAGS = -I m4m

# Help the maintainer test and publish
upload.stamp: cvs-tag.stamp
	chmod 0644 $(distdir).tar.bz2
	rsync -avz --bwlimit=14 --progress \
		$(distdir).tar.bz2 \
		sfcfs:batches/queued/
	rsync -avz --bwlimit=14 --progress \
		$(distdir).tar.bz2 \
		h:es/
	true > $@

cvs-tag.stamp: dist-bzip2
	@pkg="$$(echo "$(PACKAGE_TARNAME)" | sed 'y|.-|__|')"; \
	ver="$$(echo "$(PACKAGE_VERSION)" | sed 'y|.-|__|')"; \
	tag="$${pkg}-$${ver}-release"; \
	echo "Press ENTER to run \`cvs tag $$tag', Ctrl-C to abort."; \
	read; \
	cd "$(srcdir)" && cvs tag -c "$$tag"; \
	echo "Due to a CVS bug, you'll have to manually confirm that you want to upload."; \
	echo "Press ENTER to upload, Ctrl-C to abort."; \
	read

%.sh: %.in Makefile
	@echo "Creating $@ from $< by substitution"
	@sed -e 's|@bindir\@|$(bindir)|g' < "$<" > "$@.tmp"
	@if grep '@[A-Za-z0-9_-]@' $@.tmp > /dev/null; then \
		echo "Remaining substitutions in $@.tmp:"; \
		grep '@[A-Za-z0-9_-]@' "$@.tmp"; \
	else \
		mv -f "$@.tmp" "$@"; \
		chmod +x "$@"; \
	fi

public: upload.stamp

CLEANFILES = upload.stamp cvs-tag.stamp
