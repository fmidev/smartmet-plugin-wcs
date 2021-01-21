%define DIRNAME wcs
%define SPECNAME smartmet-plugin-%{DIRNAME}
Summary: SmartMet WCS plugin
Name: %{SPECNAME}
Version: 20.10.23
Release: 1%{?dist}.fmi
License: FMI
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-wcs
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: boost169-devel
BuildRequires: ctpp2-devel
BuildRequires: jsoncpp-devel
BuildRequires: libconfig-devel >= 1.7.2
BuildRequires: libcurl-devel
BuildRequires: libpqxx-devel < 1:7.0
BuildRequires: netcdf-cxx-devel
BuildRequires: postgresql12-libs
BuildRequires: smartmet-engine-gis-devel >= 21.1.14
BuildRequires: smartmet-engine-querydata-devel >= 21.1.14
BuildRequires: smartmet-library-gis-devel >= 21.1.14
BuildRequires: smartmet-library-macgyver-devel >= 21.1.14
BuildRequires: smartmet-library-newbase-devel >= 21.1.14
BuildRequires: smartmet-library-spine-devel >= 21.1.14
BuildRequires: xerces-c-devel
BuildRequires: xqilla-devel
BuildRequires: bzip2-devel
BuildRequires: openssl-devel
Requires: boost169-chrono
Requires: boost169-date-time
Requires: boost169-filesystem
Requires: boost169-iostreams
Requires: boost169-regex
Requires: boost169-serialization
Requires: boost169-system
Requires: boost169-thread
Requires: ctpp2
Requires: jsoncpp
Requires: libconfig >= 1.7.2
Requires: libcurl
Requires: libpqxx < 1:7.0
Requires: netcdf-cxx
Requires: smartmet-engine-gis >= 21.1.14
Requires: smartmet-engine-querydata >= 21.1.14
Requires: smartmet-library-macgyver >= 21.1.14
Requires: smartmet-library-newbase >= 21.1.14
Requires: smartmet-library-spine >= 21.1.14
Requires: smartmet-server >= 21.1.14
Requires: xerces-c
Requires: xqilla
Provides: %{SPECNAME}
#TestRequires: libpqxx-devel < 1:7.0
#TestRequires: libconfig-devel >= 1.7.2
#TestRequires: jsoncpp-devel
#TestRequires: smartmet-engine-querydata >= 21.1.14
#TestRequires: smartmet-engine-gis >= 21.1.14
#TestRequires: smartmet-library-macgyver-devel >= 21.1.14
#TestRequires: smartmet-test-data

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
* Fri Oct 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.23-1.fmi
- Use new TemplateFormatter API

* Tue Oct 20 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.20-1.fmi
- Rebuild due to libconfig upgrade to version 1.7.2

* Wed Sep 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.23-1.fmi
- Use Fmi::Exception instead of Spine::Exception

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Fri May  8 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.5.8-1.fmi
- Use CRSRegistry from smartmet-library-spine

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgraded to Boost 1.69

* Wed Nov 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.20-1.fmi
- Rebuilt due to newbase API changes

* Thu Oct 31 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.31-1.fmi
- Rebuilt due to newbase API/ABI changes

* Thu Sep 26 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.26-1.fmi
- Repackaged due to ABI changes

* Mon Nov 12 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.12-1.fmi
- Use TemplateFactory for thread safety

* Thu Aug 30 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.30-1.fmi
- dataSetDir and netcdfParamConfigPath can now be relative paths

* Mon Aug 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.13-1.fmi
- Repackaged since Spine::Location size changed

* Thu Jul 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.26-1.fmi
- Prefer nullptr over NULL

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
