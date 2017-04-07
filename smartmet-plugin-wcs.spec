%define DIRNAME wcs
%define SPECNAME smartmet-plugin-%{DIRNAME}
Summary: SmartMet WCS plugin
Name: %{SPECNAME}
Version: 17.2.11
Release: 1%{?dist}.fmi
License: FMI
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-wcs
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: boost-devel
BuildRequires: ctpp2-devel
BuildRequires: jsoncpp-devel
BuildRequires: libconfig-devel
BuildRequires: libcurl-devel
BuildRequires: libpqxx-devel
BuildRequires: netcdf-cxx-devel
BuildRequires: postgresql93-libs
BuildRequires: smartmet-engine-geonames-devel >= 17.2.3
BuildRequires: smartmet-engine-gis-devel >= 17.1.4
BuildRequires: smartmet-engine-querydata-devel >= 17.2.11
BuildRequires: smartmet-library-gis-devel >= 17.1.18
BuildRequires: smartmet-library-locus-devel >= 17.2.3
BuildRequires: smartmet-library-macgyver-devel >= 17.1.18
BuildRequires: smartmet-library-spine-devel >= 17.2.3
BuildRequires: xerces-c-devel
BuildRequires: xqilla-devel
Requires: boost-chrono
Requires: boost-date-time
Requires: boost-filesystem
Requires: boost-iostreams
Requires: boost-regex
Requires: boost-serialization
Requires: boost-system
Requires: boost-thread
Requires: ctpp2
Requires: jsoncpp
Requires: libconfig
Requires: libcurl
Requires: libpqxx
Requires: netcdf-cxx
Requires: smartmet-engine-geonames >= 17.2.3
Requires: smartmet-engine-gis >= 17.1.4
Requires: smartmet-engine-gis >= 17.1.4
Requires: smartmet-engine-querydata >= 17.2.11
Requires: smartmet-library-locus >= 17.2.3
Requires: smartmet-library-macgyver >= 17.1.18
Requires: smartmet-library-spine >= 17.2.3
Requires: smartmet-server >= 17.1.25
Requires: xerces-c
Requires: xqilla
Provides: %{SPECNAME}

%description
SmartMet WCS plugin

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n plugins/%{SPECNAME}

%build -n plugins/%{SPECNAME}
make %{_smp_mflags}

%install
%makeinstall
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/plugins
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/templates
for file in cnf/templates/*.c2t; do
    install -m 664 $file  $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/templates/
done

install -m 664 cnf/XMLGrammarPool.dump $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/
install -m 664 cnf/XMLSchemas.cache $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0775,root,root,0775)
%{_datadir}/smartmet/plugins/%{DIRNAME}.so
%defattr(0664,root,root,0775)
%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/templates/*.c2t
%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/XMLGrammarPool.dump
%{_sysconfdir}/smartmet/plugins/%{DIRNAME}/XMLSchemas.cache

%changelog
* Upcoming
- Autoremove data from the path of UniqueTemporaryPath after instance lifetime.
- Check that subsetting is placed to the correct region of a data.
- Added multiple regression tests to test subsetting.
- Fixed a bug in UniqueTemporaryPath class.

* Sat Feb 11 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.11-1.fmi
- Repackaged due to newbase API changes

* Thu Feb  2 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.2-1.fmi
- Unified handling of apikeys

* Mon Jan 30 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.30-1.fmi
- Fixes to NetCDF filename generation, the length limit is 36 characters

* Fri Jan 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.27-1.fmi
- Changed to use the new type of NFmiQueryData latlon-cache

* Wed Jan  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.4-1.fmi
- Changed to use renamed SmartMet base libraries

* Wed Nov 30 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.30-1.fmi
- New release

* Mon Nov 28 2016 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 16.11.28-1.fmi
- Test build

* Thu Jan 28 2016 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 16.1.28-1.fmi
- First build
