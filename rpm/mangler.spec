# norootforbuild

Name:		mangler
Version:	1.2.5
Release:	1
Summary:	Mangler is a Ventrilo compatible client for Linux

Group:		Productivity/Networking/Talk/Clients
License:	GPL
URL:		http://www.mangler.org/
Source0:	%{name}-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-build

BuildRequires:	speex-devel espeak-devel xosd-devel gcc-c++
%if 0%{?suse_version}
BuildRequires:  gtkmm2-devel libgsm-devel libpulse-devel alsa-devel dbus-1-glib-devel update-desktop-files librsvg
%endif

%if 0%{?fedora_version}
BuildRequires:  gtkmm24-devel gsm-devel pulseaudio-libs-devel alsa-lib-devel libgdbus-devel librsvg2
#Requires:       gtkmm24 speex gsm pulseaudio-libs espeak libgdbus xosd
%endif

%description
Mangler is a VOIP client that is capable of connecting to Ventrilo 3.x servers

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT
rm -f "%{buildroot}%{_prefix}/include/ventrilo3.h"
rm -f "%{buildroot}%{_libdir}/libventrilo3.a" "%{buildroot}%{_libdir}/libventrilo3.la" "%{buildroot}%{_libdir}/libventrilo3.so"
%if 0%{?suse_version}
%suse_update_desktop_file mangler Telephony
%endif

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/mangler
#%{_libdir}/libventrilo3.a
#%{_libdir}/libventrilo3.la
#%{_libdir}/libventrilo3.so
%{_libdir}/libventrilo3.so.0
%{_libdir}/libventrilo3.so.0.0.0
%{_datadir}/applications/mangler.desktop
%{_datadir}/pixmaps/mangler_logo.svg
#%{_prefix}/include/ventrilo3.h
%doc

%changelog

