%define ver	0.3.1
%define rel	2
%define prefix	/usr

Summary: GNU Photo (gphoto)
Name: gphoto        
Version: %ver
Release: %rel
Copyright: GPL
Group: X11/Utilities
Source: http://www.mustec.eu.org/~psj/downloads/gphoto-%{ver}.tar.gz
BuildRoot: /var/tmp/gphoto-root
URL: http://www.gphoto.org/
Docdir: %{prefix}/doc
Requires: gtk+ >= 1.2.0

%description
gphoto is part of the GNOME project and is
a fine interface for a wide variety of digital
cameras

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
make

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} install

%post
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS COPYING CREDITS ChangeLog FAQ MANUAL NEWS README THEMES
%{prefix}/man/man1/gphoto.1
%{prefix}/bin/*
%{prefix}/share/gphoto/drivers/*
%{prefix}/share/gphoto/gallery/Default/*
%{prefix}/share/gphoto/gallery/RedNGray/*

