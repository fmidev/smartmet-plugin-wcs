%define DIRNAME wcs
%define SPECNAME smartmet-plugin-%{DIRNAME}
Summary: SmartMet WCS plugin
Name: %{SPECNAME}
Version: 18.4.25
Release: 1%{?dist}.fmi
License: FMI
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-wcs
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: boost-devel
BuildRequires: ctpp2-devel
BuildRequires: jsoncpp-devel
BuildRequires: libconfig-devel
BuildRequires: libcurl-devel
BuildRequires: libpqxx-devel
BuildRequires: netcdf-cxx-devel
BuildRequires: postgresql95-libs
BuildRequires: smartmet-engine-gis-devel >= 18.4.7
BuildRequires: smartmet-engine-querydata-devel >= 18.4.7
BuildRequires: smartmet-library-gis-devel >= 18.4.7
BuildRequires: smartmet-library-macgyver-devel >= 18.4.7
BuildRequires: smartmet-library-newbase-devel >= 18.4.7
BuildRequires: smartmet-library-spine-devel >= 18.4.7
BuildRequires: xerces-c-devel
BuildRequires: xqilla-devel
BuildRequires: bzip2-devel
BuildRequires: openssl-devel
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
Requires: smartmet-engine-gis >= 18.4.7
Requires: smartmet-engine-gis >= 18.4.7
Requires: smartmet-engine-querydata >= 18.4.7
Requires: smartmet-library-macgyver >= 18.4.7
Requires: smartmet-library-newbase >= 18.4.7
Requires: smartmet-library-spine >= 18.4.7
Requires: smartmet-server >= 18.4.7
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
* Wed Apr 25 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.25-1.fmi
- Split setCoverageIds and fix outputCrs support

* Sat Apr  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.7-1.fmi
- Upgrade to boost 1.66

* Tue Mar 20 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.20-1.fmi
- Full recompile of all server plugins

* Wed Mar  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.7-1.fmi
- Fixed axis label ordering

* Tue Feb 27 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.27-1.fmi
- Metadata URLs are now more configurable

* Wed Nov  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.1-2.fmi
- Rebuilt due to GIS-library API change

* Wed Nov  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.1-1.fmi
- WGS84BoundingBox fixes

* Fri Oct 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.10.27-2.fmi
- Fixed capabilities template

* Fri Oct 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.10.27-1.fmi
- Added service metadata into the main configuration file
- Added multilanguage support into the configuration files

* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65
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
