# $Id$
#
# $Log$
# Revision 1.1  1999/05/27 18:32:01  scottf
# Initial revision
#
# Revision 1.5  1999/05/08 22:38:51  gdr
# Forgot to put dependency of dist-clean on target distclean
#
# Revision 1.4  1999/05/08 18:08:24  gdr
# Modified to hopefully make more portable
#
# Revision 1.3  1999/05/05 19:50:21  ole
# Initial revision
#

gphoto:
	make -C src

clean:
	make -C src clean

cleanall:
	make -C directory clean
	make -C photopc clean
	make -C fuji clean
	make -C kodak clean 
	make -C konica clean
	make -C philips clean
	make -C ricoh clean
	make -C sony clean
	make -C casio clean
	make -C src clean

dist-clean:
	make -C src dist-clean

distclean: dist-clean

install:
	make -C src install

uninstall:
	make -C src uninstall

