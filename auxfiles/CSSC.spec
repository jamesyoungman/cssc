Summary: GNU CSSC - An SCCS clone
Name: CSSC
Version: 0.06alpha.pl7
Release: 1
URL: http://www.myth.co.uk/~jay/cssc/
Copyright: GPL
Packager: James Youngman <jay@gnu.org>
Group: Development/Version Control
Source: ftp://alpha.gnu.org:/pub/gnu/CSSC/CSSC-0.06alpha.pl7.tar.gz
Requires: /usr/bin/diff
BuildRoot: /var/tmp/cssc-root

%description
CSSC is a clone for the traditional Unix version control suite SCCS.
It aims for near-total compatibility with SCCS.  CSSC stands for
"Compatibly Stupid Source Control".  The BSD driver program sccs(1) is
not included.  You can get that from ftp.freebsd.org.

%prep
%setup

%build
rm -rf $RPM_BUILD_ROOT
rm -f docs/cssc.info
./configure --prefix=/usr --infodir=/usr/info
make

%install
make prefix=$RPM_BUILD_ROOT/usr install-strip
# [ -L /usr/sccs ] && rm -f /usr/sccs
ln -s libexec/cssc $RPM_BUILD_ROOT/usr/sccs

%post
/sbin/install-info /usr/info/cssc.info /usr/info/dir

%preun
if [ $1 = 0 ]; then
	/sbin/install-info --delete /usr/info/cssc.info /usr/info/dir
fi

%clean
rm -rf $RPM_BUILD_ROOT



%changelog

* Fri May  8 1998 interran@crd.GE.COM <John Interrante>
Use a build-root.   Also use install-info.  Use install-strip 
rather than just "strip *".

* Sat Feb 21 1998 jay@gnu.org <James Youngman>
Strip the installed binaries.

* Thu Feb 12 1998 jay@gnu.org <James Youngman>
Added sccsdiff to the file list.

* Sat Jan 17 1998 jay@gnu.org <James Youngman>

First RPMed version (0.05alpha-pl0)

%files
%attr(-, root, root) %doc README AUTHORS COPYING ChangeLog INSTALL NEWS 
%attr(-, root, root) %doc docs/BUGS docs/CREDITS docs/FIXED docs/Platforms 
%attr(-, root, root) %doc docs/TESTING docs/TODO docs/missing.txt docs/patches.txt
%attr(-, root, root) %doc /usr/info/cssc.info
%attr(-, root, root) %doc /usr/info/cssc.info-1
%attr(-, root, root) %doc /usr/info/cssc.info-2
%attr(755, root, root) /usr/libexec/cssc
%attr(-, root, root) /usr/sccs
