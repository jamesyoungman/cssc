Summary: GNU CSSC - An SCCS clone
Name: CSSC
Version: 0.06alpha_pl0
Release: 1
Copyright: GPL
Packager: James Youngman <jay@gnu.org>
Group: Development/Version Control
Source: alpha.gnu.org:/pub/gnu/CSSC/CSSC-0.05alpha-pl0.tar.gz 
Requires: /usr/bin/diff

%description
CSSC is a clone for the traditional Unix version control suite SCCS.
It aims for near-total compatibility with SCCS.  CSSC stands for
"Compatibly Stupid Source Control".  The BSD driver program sccs(1) is
not included.  You can get that from ftp.freebsd.org.

%prep
%setup -n CSSC-0.05alpha-pl0

%build
./configure --prefix=/usr --infodir=/usr/info
make

%install
make install
[ -L /usr/sccs ] && rm -f /usr/sccs
ln -s libexec/cssc /usr/sccs

%changelog

* Thu Feb 12 1998 jay@gnu.org <James Youngman>
Added sccsdiff to the file list.

* Sat Jan 17 1998 jay@gnu.org <James Youngman>

First RPMed version (0.05alpha-pl0)

%files
%doc README AUTHORS COPYING ChangeLog INSTALL NEWS 
%doc docs/BUGS docs/CREDITS docs/FIXED docs/Platforms 
%doc docs/TESTING docs/TODO docs/missing.txt docs/patches.txt
%doc /usr/info/cssc.info
%doc /usr/info/cssc.info-1
%doc /usr/info/cssc.info-2
/usr/libexec/cssc/admin 
/usr/libexec/cssc/cdc 
/usr/libexec/cssc/delta 
/usr/libexec/cssc/get 
/usr/libexec/cssc/prs 
/usr/libexec/cssc/prt
/usr/libexec/cssc/rmdel 
/usr/libexec/cssc/sact 
/usr/libexec/cssc/unget 
/usr/libexec/cssc/what 
/usr/libexec/cssc/sccsdiff
/usr/sccs
