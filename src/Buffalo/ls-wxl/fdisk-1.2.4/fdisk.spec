Name: fdisk
Version: 0.9
Release: 1

License: GPL
Group: File tools
Summary: GNU fdisk 
URL: http://svn.debian.org/wsvn/parted/upstream/people/leslie/fdisk/?rev=0&sc=0

Source: %name-%version.tar.gz

BuildRequires: libparted-devel >= 1.7.0 libe2fs-devel

%description
Clone of Linux fdisk with libparted as backend.

%prep
%setup -q

%build
autoreconf -fisv
%configure
%make_build

%install
%makeinstall install

%files
%_sbindir/%name

%changelog
* Sat Mar 11 2006 polzer@gnu.org 0.9
- conversion to stand-alone package.

