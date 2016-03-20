%define reldate 20160320
%define reltype C
# may be one of: C (current), R (release), S (stable)

Name: fidoconf
Version: 1.9.%{reldate}%{reltype}
Release: 1
Group: Libraries/FTN
Summary: Common FTN configuration library
URL: http://husky.sf.net
License: GPL
Requires: perl >= 5.8.8
BuildRequires: huskylib >= 1.9, smapi >= 2.5
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Common FTN configuration library for the Husky Project software.

%package utils
Group: Libraries/FTN
Summary: Utilities for %{name}

%description utils
Utilities for configuration library of the Husky Project software.
%summary

%prep
%setup -q -n %{name}

%build
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
chmod -R a+rX,u+w,go-w %{buildroot}

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_includedir}/%{name}/*
%{_libdir}/*.a

%files utils
%defattr(-,root,root)
%{_bindir}/*
