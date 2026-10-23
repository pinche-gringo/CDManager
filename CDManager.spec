# RPM spec file for CDManager
#
# The version is passed from outside (it's maintained in meson.build only):
#   rpmbuild -ba --define "cdmgr_version 1.2.00" CDManager.spec
# Optionally the needed version of libYGP/libXGP (ygp_version in meson.build)
# can be passed as well with --define "ygp_version 0.9.70".
# with CDManager-<version>.tar.xz (from "meson dist") in the SOURCES directory.

%{!?cdmgr_version: %{error:Define cdmgr_version (e.g. rpmbuild --define "cdmgr_version 1.2.00")}}

Name:           CDManager
Version:        %{cdmgr_version}
Release:        1%{?dist}
Summary:        Manages records, films and actors in a database
License:        GPL-3.0-or-later
URL:            https://github.com/pinche-gringo/CDManager
Source0:        %{name}-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  gcc-c++ >= 16
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  gettext
BuildRequires:  boost-devel
BuildRequires:  gtkmm4.0-devel
BuildRequires:  libsoup3-devel
BuildRequires:  libpq-devel
BuildRequires:  libYGP-devel%{?ygp_version: >= %{ygp_version}}
BuildRequires:  libXGP-devel%{?ygp_version: >= %{ygp_version}}

%description
CDManager is a GTK 4 application to manage a collection of records
(with their songs), films and actors stored in a PostgreSQL database.
Film information can be imported from IMDb and the collection can be
exported as HTML.

%prep
%autosetup

%build
%meson -Ddatabase=postgresql -Dnls=enabled
%meson_build

%install
%meson_install
%find_lang %{name}
# The executables have an RPATH to the (standard) library directory
export QA_RPATHS=0x0001

%check
%meson_test

%files -f %{name}.lang
%license COPYING
%doc AUTHORS NEWS README doc/CDManager.html.*
%{_bindir}/CDManager
%{_bindir}/CDWriter
%{_libdir}/libCDMgr.so
%{_libdir}/libCDMgr.so.0
%{_libdir}/libCDMgr.so.0.0.0
%{_datadir}/%{name}/
%{_datadir}/applications/CDManager.desktop
%{_datadir}/icons/CDManager.png

%changelog
