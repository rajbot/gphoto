SUBDIRS = dist-files utils

EXTRA_DIST = README bootstrap.sh clean.sh autogen.sh compileinstall.sh cvs-module-list cvs-module-list-candidates build-tool-list

install-exec-local: buildstuff

.PHONY: buildstuff
buildstuff:
	echo "BLAF"

# Note that creating bz2 distribution tarballs doesn't make sense.
# The tarballs in dist/ are so much precompressed data that the
# gzipped tarball of gphoto-meta is smaller than the bzip2ed one.
