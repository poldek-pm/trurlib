# $Revision$, $Date$
Summary:	trurlib 
Name:		trurlib
Version:	0.4
Release:	1
License:	LGPL
Group:		Development/Libraries
Group(pl):	Programowanie/Biblioteki
Source:		%{name}-%{version}.tar.gz
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

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
make static CFLAGS='-O2 -fomit-frame-pointer -DNDEBUG'
make shared without_dbhash=1 CFLAGS='-O2 -fomit-frame-pointer -DNDEBUG'
 
%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_prefix}
make install prefix=$RPM_BUILD_ROOT%{_prefix}

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
Revision 1.1  2000/06/09 16:37:19  mis
- spec

