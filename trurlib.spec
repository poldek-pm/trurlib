# $Revision$, $Date$
Summary:	trurlib 
Name:		trurlib
Version:	0.42
Release:	1
License:	LGPL
Group:		Development/Libraries
Group(pl):	Programowanie/Biblioteki
Source:		%{name}-%{version}.tar.gz
BuildRoot:	/tmp/%{name}-%{version}-root-%(id -u -n)

%description
trurlib

%package	devel
Summary:	Static library and header files of trurlib
Group:		Development/Libraries
Group(pl):	Programowanie/Biblioteki
Requires:	%{name} = %{version}

%description devel
Static library and header files of trurlib

%prep 
%setup -q 

%build
CFLAGS="$RPM_OPT_FLAGS -fomit-frame-pointer -DNDEBUG"
make static modules=on CFLAGS="$CFLAGS"
make shared modules=on without_dbhash=1 CFLAGS="$CFLAGS"
 
%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_prefix}
make install DESTDIR=$RPM_BUILD_ROOT%{_prefix}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_libdir}/lib*.so.*

%files devel
%defattr(644,root,root,755)
%attr(644,root,root) %{_libdir}/lib*.a
%attr(644,root,root) %{_libdir}/lib*.so
%{_includedir}/trurl


%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%define date    %(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} PLD Team <pld-list@pld.org.pl>
All persons listed below can be reached at <cvs_login>@pld.org.pl

$Log$
Revision 1.5  2000/09/20 18:33:51  mis
- %{tmpdir} -> /tmp, distribution spec shouldn't be PLD dependent

Revision 1.4  2000/09/20 16:30:26  mis
- current working version is v0.42

Revision 1.3  2000/07/20 15:33:05  mis
- version 0.41

Revision 1.2  2000/07/15 12:39:00  mis
- modularized narray, nhash and nlist modules

Revision 1.1  2000/06/09 16:37:19  mis
- spec

