## NOTE: Lots of files in various subdirectories have the same name (such as
## "LICENSE") so this short macro allows us to distinguish them by using their
## directory names (from the source tree) as prefixes for the files.
%global add_to_license_files() \
        mkdir -p _license_files ; \
        cp -p %1 _license_files/$(echo '%1' | sed -e 's!/!.!g')

# Bundle ICU 57 - see https://bugzilla.redhat.com/show_bug.cgi?id=1414413
%define bundle_icu 1
%if 0%{?bundle_icu}
# Don't list bundled icu libraries in provides
%global __provides_exclude %{?__provides_exclude:%__provides_exclude|}libicu.*
%global __requires_exclude %{?__provides_exclude:%__provides_exclude|}libicu.*
%endif

# As we are using the DTS we have to build it as:
# rhpkg build --target devtoolset-6-gnome-rhel-7.4-candidate

Name:           webkitgtk4
Version:        2.14.7
Release:        3%{?dist}
Summary:        GTK+ Web content engine library

License:        LGPLv2
URL:            http://www.webkitgtk.org
Source0:        http://webkitgtk.org/releases/webkitgtk-%{version}.tar.xz
%if 0%{?bundle_icu}
Source1:        http://download.icu-project.org/files/icu4c/57.1/icu4c-57_1-src.tgz
%endif

# https://bugs.webkit.org/show_bug.cgi?id=156596
Patch0:         inttypes_prid64.patch
# Covscan fixes
Patch1:         covscan.patch
# https://bugs.webkit.org/show_bug.cgi?id=165923
Patch2:         b165923.patch
# https://bugs.webkit.org/show_bug.cgi?id=167845
Patch3:         b167845.patch
# https://bugs.webkit.org/show_bug.cgi?id=169027
Patch4:         b169027.patch
# https://bugs.webkit.org/show_bug.cgi?id=169028
Patch5:         b169028.patch
# https://bugs.webkit.org/show_bug.cgi?id=169029
Patch6:         b169029.patch
# https://bugs.webkit.org/show_bug.cgi?id=169035
Patch7:         b169035.patch
# https://bugs.webkit.org/show_bug.cgi?id=169055
Patch8:        b169055.patch
# https://bugs.webkit.org/show_bug.cgi?id=169597
Patch9:        b169597.patch
# https://bugs.webkit.org/show_bug.cgi?id=169598
Patch10:        b169598.patch
# https://bugs.webkit.org/show_bug.cgi?id=169602
Patch11:        b169602.patch
# https://bugs.webkit.org/show_bug.cgi?id=169604
Patch12:        b169604.patch
# https://bugs.webkit.org/show_bug.cgi?id=169610
Patch13:        b169610.patch
# https://bugs.webkit.org/show_bug.cgi?id=169665
Patch14:        b169665.patch
# https://bugs.webkit.org/show_bug.cgi?id=169666
Patch15:        b169666.patch
# https://bugs.webkit.org/show_bug.cgi?id=170448
Patch16:        b170448.patch
# https://bugs.webkit.org/show_bug.cgi?id=167691
Patch17:        b167691.patch
# https://bugs.webkit.org/show_bug.cgi?id=171429
Patch18:        b171429.patch
# https://bugs.webkit.org/show_bug.cgi?id=171443
Patch19:        b171443.patch
# https://bugs.webkit.org/show_bug.cgi?id=171768
Patch20:        b171768.patch
# https://bugs.webkit.org/show_bug.cgi?id=171546
Patch21:        b171546.patch
# https://bugs.webkit.org/show_bug.cgi?id=171547
Patch22:        b171547.patch
# https://bugs.webkit.org/show_bug.cgi?id=171927
Patch23:        b171927.patch
# https://bugs.webkit.org/show_bug.cgi?id=170945
Patch24:        b170945.patch
# https://bugs.webkit.org/show_bug.cgi?id=132333
Patch25:        b132333.patch
# https://bugs.webkit.org/show_bug.cgi?id=162608
Patch26:        webkit-update-bundled-woff2.patch
# https://bugs.webkit.org/show_bug.cgi?id=177768
Patch27:        webkit-update-bundled-brotli-and-woff2.patch
# https://bugs.webkit.org/show_bug.cgi?id=177994
Patch28:        webkit-relicense-bundled-woff2-to-mit.patch
# https://github.com/google/woff2/pull/96
Patch29:        webkit-woff2-cassert-include.patch

%if 0%{?bundle_icu}
Patch50: icu-8198.revert.icu5431.patch
Patch51: icu-8800.freeserif.crash.patch
Patch52: icu-7601.Indic-ccmp.patch
Patch53: icu-gennorm2-man.patch
Patch54: icu-icuinfo-man.patch
Patch55: icu-armv7hl-disable-tests.patch
Patch56: icu-rhbz1360340-icu-changeset-39109.patch
Patch57: icu-diff-icu_trunk_source_common_locid.cpp-from-39282-to-39384.patch
Patch58: icu-dont_use_clang_even_if_installed.patch
%endif

BuildRequires:  at-spi2-core-devel
BuildRequires:  bison
BuildRequires:  cairo-devel
BuildRequires:  cmake
BuildRequires:  enchant-devel
BuildRequires:  flex
BuildRequires:  fontconfig-devel
BuildRequires:  freetype-devel
BuildRequires:  geoclue2-devel
BuildRequires:  gettext
BuildRequires:  glib2-devel
BuildRequires:  gobject-introspection-devel
BuildRequires:  gperf
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-base-devel
BuildRequires:  gtk2-devel
BuildRequires:  gtk3-devel
BuildRequires:  gtk-doc >= 1.25
BuildRequires:  harfbuzz-devel
%if ! 0%{?bundle_icu}
BuildRequires:  libicu-devel
%endif
BuildRequires:  libjpeg-devel
BuildRequires:  libnotify-devel
BuildRequires:  libpng-devel
BuildRequires:  libsecret-devel
BuildRequires:  libsoup-devel >= 2.56
BuildRequires:  libwebp-devel
BuildRequires:  libxslt-devel
BuildRequires:  libXt-devel
BuildRequires:  mesa-libGL-devel
BuildRequires:  pcre-devel
BuildRequires:  perl-Switch
BuildRequires:  ruby
BuildRequires:  rubygems
BuildRequires:  sqlite-devel
BuildRequires:  hyphen-devel
BuildRequires:  gnutls-devel
%ifarch ppc
BuildRequires:  libatomic
%endif
BuildRequires: devtoolset-6-gcc
BuildRequires: devtoolset-6-gcc-c++
BuildRequires: devtoolset-6-build

Requires:       geoclue2

%if 0%{?bundle_icu}
BuildRequires: doxygen
BuildRequires: autoconf
BuildRequires: python
%endif

# Obsolete libwebkit2gtk from the webkitgtk3 package
Obsoletes:      libwebkit2gtk < 2.5.0
Provides:       libwebkit2gtk = %{version}-%{release}

# We're supposed to specify versions here, but these Google libs don't do
# normal releases. Accordingly, they're not suitable to be system libs.
# Provides:       bundled(angle)
# Provides:       bundled(brotli)
# Provides:       bundled(woff2)

# Require the jsc subpackage
Requires:       %{name}-jsc%{?_isa} = %{version}-%{release}

# Require the support for the GTK+ 2 based NPAPI plugins
# Would be nice to recommend as in Fedora, but RHEL7 RPM doesn't support it.
Requires:       %{name}-plugin-process-gtk2%{?_isa} = %{version}-%{release}

# Filter out provides for private libraries
%global __provides_exclude_from ^%{_libdir}/webkit2gtk-4\\.0/.*\\.so$

%description
WebKitGTK+ is the port of the portable web rendering engine WebKit to the
GTK+ platform.

This package contains WebKitGTK+ for GTK+ 3.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       %{name}-jsc%{?_isa} = %{version}-%{release}
Requires:       %{name}-jsc-devel%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries, build data, and header
files for developing applications that use %{name}.

%package        doc
Summary:        Documentation files for %{name}
BuildArch:      noarch
Requires:       %{name} = %{version}-%{release}

%description    doc
This package contains developer documentation for %{name}.

%package        jsc
Summary:        JavaScript engine from %{name}
Requires:       %{name} = %{version}-%{release}

%description    jsc
This package contains JavaScript engine from %{name}.

%package        jsc-devel
Summary:        Development files for JavaScript engine from %{name}
Requires:       %{name}-jsc%{?_isa} = %{version}-%{release}
Requires:       %{name} = %{version}-%{release}

%description    jsc-devel
The %{name}-jsc-devel package contains libraries, build data, and header
files for developing applications that use JavaScript engine from %{name}.

%package        plugin-process-gtk2
Summary:        GTK+ 2 based NPAPI plugins support for %{name}
Requires:       %{name}-jsc%{?_isa} = %{version}-%{release}
Requires:       %{name} = %{version}-%{release}

%description    plugin-process-gtk2
Support for the GTK+ 2 based NPAPI plugins (such as Adobe Flash) for %{name}.

%prep
%if 0%{?bundle_icu}
%setup -q -T -n icu -b 1
%patch50 -p2 -R -b .icu8198.revert.icu5431.patch
%patch51 -p1 -b .icu8800.freeserif.crash.patch
%patch52 -p1 -b .icu7601.Indic-ccmp.patch
%patch53 -p1 -b .gennorm2-man.patch
%patch54 -p1 -b .icuinfo-man.patch
%ifarch armv7hl
%patch55 -p1 -b .armv7hl-disable-tests.patch
%endif
%patch56 -p1 -b .rhbz1360340-icu-changeset-39109.patch
%patch57 -p1 -b .diff-icu_trunk_source_common_locid.cpp-from-39282-to-39384.patch
%patch58 -p1 -b .dont_use_clang_even_if_installed

%setup -q -T -n webkitgtk-%{version} -b 0
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1
%patch4 -p1
%patch5 -p1
%patch6 -p1
%patch7 -p1
%patch8 -p1
%patch9 -p1
%patch10 -p1
%patch11 -p1
%patch12 -p1
%patch13 -p1
%patch14 -p1
%patch15 -p1
%patch16 -p1
%patch17 -p1 -b .b167691
%patch18 -p1 -b .b171429
%patch19 -p1 -b .b171443
%patch20 -p1 -b .b171768
%patch21 -p1 -b .b171546
%patch22 -p1 -b .b171547
%patch23 -p1 -b .b171927
%patch24 -p1 -b .b170945
%patch25 -p1 -b .b132333
%patch26 -p1
%patch27 -p1
%patch28 -p1
%patch29 -p1
%else
%autosetup -p1 -n webkitgtk-%{version}
%endif

# Remove bundled libraries
rm -rf Source/ThirdParty/gtest/
rm -rf Source/ThirdParty/qunit/

%build
%ifarch s390 aarch64
# Use linker flags to reduce memory consumption - on other arches the ld.gold is
# used and also it doesn't have the --reduce-memory-overheads option
%global optflags %{optflags} -Wl,--no-keep-memory -Wl,--reduce-memory-overheads
%endif

# Decrease debuginfo even on ix86 because of:
# https://bugs.webkit.org/show_bug.cgi?id=140176
%ifarch s390 s390x %{arm} %{ix86} ppc %{power64} %{mips}
# Decrease debuginfo verbosity to reduce memory consumption even more
%global optflags %(echo %{optflags} | sed 's/-g /-g1 /')
%endif

%ifarch ppc
# Use linker flag -relax to get WebKit build under ppc(32) with JIT disabled
%global optflags %{optflags} -Wl,-relax
%endif

%if 0%{?bundle_icu}
pushd ../icu/source
autoconf
CFLAGS='%optflags -fno-strict-aliasing'
CXXFLAGS='%optflags -fno-strict-aliasing'
%{!?endian: %global endian %(%{__python} -c "import sys;print (0 if sys.byteorder=='big' else 1)")}
# " this line just fixes syntax highlighting for vim that is confused by the above and continues literal
# Endian: BE=0 LE=1
%if ! 0%{?endian}
CPPFLAGS='-DU_IS_BIG_ENDIAN=1'
%endif

#rhbz856594 do not use --disable-renaming or cope with the mess
OPTIONS='--with-data-packaging=library --disable-samples'
%configure $OPTIONS

#rhbz#225896
sed -i 's|-nodefaultlibs -nostdlib||' config/mh-linux
#rhbz#681941
sed -i 's|^LIBS =.*|LIBS = -L../lib -licuuc -lpthread -lm|' i18n/Makefile
sed -i 's|^LIBS =.*|LIBS = -nostdlib -L../lib -licuuc -licui18n -lc -lgcc|' io/Makefile
sed -i 's|^LIBS =.*|LIBS = -nostdlib -L../lib -licuuc -lc|' layout/Makefile
sed -i 's|^LIBS =.*|LIBS = -nostdlib -L../lib -licuuc -licule -lc|' layoutex/Makefile
sed -i 's|^LIBS =.*|LIBS = -nostdlib -L../../lib -licutu -licuuc -lc|' tools/ctestfw/Makefile
# As of ICU 52.1 the -nostdlib in tools/toolutil/Makefile results in undefined reference to `__dso_handle'
sed -i 's|^LIBS =.*|LIBS = -L../../lib -licui18n -licuuc -lpthread -lc|' tools/toolutil/Makefile
#rhbz#813484
sed -i 's| \$(docfilesdir)/installdox||' Makefile
# There is no source/doc/html/search/ directory
sed -i '/^\s\+\$(INSTALL_DATA) \$(docsrchfiles) \$(DESTDIR)\$(docdir)\/\$(docsubsrchdir)\s*$/d' Makefile
# rhbz#856594 The configure --disable-renaming and possibly other options
# result in icu/source/uconfig.h.prepend being created, include that content in
# icu/source/common/unicode/uconfig.h to propagate to consumer packages.
test -f uconfig.h.prepend && sed -e '/^#define __UCONFIG_H__/ r uconfig.h.prepend' -i common/unicode/uconfig.h

# more verbosity for build.log
sed -i -r 's|(PKGDATA_OPTS = )|\1-v |' data/Makefile

make %{?_smp_mflags} VERBOSE=1
cd ..
BUNDLED_ICU_PATH="`pwd`/icu_installed"
make %{?_smp_mflags} -C source install DESTDIR=$BUNDLED_ICU_PATH
popd
%endif

# Enable DTS
%{?enable_devtoolset6:%{enable_devtoolset6}}

# Disable ld.gold on s390 as it does not have it.
# Also for aarch64 as the support is in upstream, but not packaged in Fedora.
mkdir -p %{_target_platform}
pushd %{_target_platform}
%cmake \
  -DPORT=GTK \
  -DCMAKE_BUILD_TYPE=Release \
%if 0%{bundle_icu}
  -DICU_DATA_LIBRARY=$BUNDLED_ICU_PATH/%{_libdir}/libicudata.so \
  -DICU_I18N_LIBRARY=$BUNDLED_ICU_PATH/%{_libdir}/libicui18n.so \
  -DICU_INCLUDE_DIR=$BUNDLED_ICU_PATH/%{_includedir} \
  -DICU_LIBRARY=$BUNDLED_ICU_PATH/%{_libdir}/libicuuc.so \
  -DCMAKE_INSTALL_RPATH=%{_libdir}/webkit2gtk-4.0 \
%endif
  -DENABLE_GTKDOC=ON \
  -DENABLE_MINIBROWSER=ON \
%ifarch s390 aarch64
  -DUSE_LD_GOLD=OFF \
%endif
%ifarch s390 s390x ppc %{power64} aarch64 %{mips}
  -DENABLE_JIT=OFF \
%endif
%ifarch s390 s390x ppc %{power64} aarch64 %{mips}
  -DUSE_SYSTEM_MALLOC=ON \
%endif
  ..
popd

make %{?_smp_mflags} -C %{_target_platform}

%install
%if 0%{?bundle_icu}
pushd ../icu/icu_installed/%{_libdir}
mkdir -p $RPM_BUILD_ROOT%{_libdir}/webkit2gtk-4.0/
cp -a libicudata.so.* $RPM_BUILD_ROOT%{_libdir}/webkit2gtk-4.0/
cp -a libicui18n.so.* $RPM_BUILD_ROOT%{_libdir}/webkit2gtk-4.0/
cp -a libicuuc.so.* $RPM_BUILD_ROOT%{_libdir}/webkit2gtk-4.0/
popd
%endif
%make_install %{?_smp_mflags} -C %{_target_platform}

%find_lang WebKit2GTK-4.0

# Finally, copy over and rename various files for %%license inclusion
%add_to_license_files Source/JavaScriptCore/COPYING.LIB
%add_to_license_files Source/JavaScriptCore/icu/LICENSE
%add_to_license_files Source/ThirdParty/ANGLE/LICENSE
%add_to_license_files Source/ThirdParty/ANGLE/src/third_party/compiler/LICENSE
%add_to_license_files Source/ThirdParty/ANGLE/src/third_party/murmurhash/LICENSE
%add_to_license_files Source/WebCore/icu/LICENSE
%add_to_license_files Source/WebCore/LICENSE-APPLE
%add_to_license_files Source/WebCore/LICENSE-LGPL-2
%add_to_license_files Source/WebCore/LICENSE-LGPL-2.1
%add_to_license_files Source/WebInspectorUI/UserInterface/External/CodeMirror/LICENSE
%add_to_license_files Source/WebInspectorUI/UserInterface/External/Esprima/LICENSE
%add_to_license_files Source/WTF/icu/LICENSE
%add_to_license_files Source/WTF/wtf/dtoa/COPYING
%add_to_license_files Source/WTF/wtf/dtoa/LICENSE

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%post jsc -p /sbin/ldconfig
%postun jsc -p /sbin/ldconfig

%files -f WebKit2GTK-4.0.lang
%license _license_files/*ThirdParty*
%license _license_files/*WebCore*
%license _license_files/*WebInspectorUI*
%license _license_files/*WTF*
%{_libdir}/libwebkit2gtk-4.0.so.*
%{_libdir}/girepository-1.0/WebKit2-4.0.typelib
%{_libdir}/girepository-1.0/WebKit2WebExtension-4.0.typelib
%{_libdir}/webkit2gtk-4.0/
%{_libexecdir}/webkit2gtk-4.0/
%exclude %{_libexecdir}/webkit2gtk-4.0/WebKitPluginProcess2

%files devel
%{_libexecdir}/webkit2gtk-4.0/MiniBrowser
%{_includedir}/webkitgtk-4.0/
%exclude %{_includedir}/webkitgtk-4.0/JavaScriptCore
%{_libdir}/libwebkit2gtk-4.0.so
%{_libdir}/pkgconfig/webkit2gtk-4.0.pc
%{_libdir}/pkgconfig/webkit2gtk-web-extension-4.0.pc
%{_datadir}/gir-1.0/WebKit2-4.0.gir
%{_datadir}/gir-1.0/WebKit2WebExtension-4.0.gir

%files jsc
%license _license_files/*JavaScriptCore*
%{_libdir}/libjavascriptcoregtk-4.0.so.*
%{_libdir}/girepository-1.0/JavaScriptCore-4.0.typelib

%files jsc-devel
%{_libexecdir}/webkit2gtk-4.0/jsc
%dir %{_includedir}/webkitgtk-4.0
%{_includedir}/webkitgtk-4.0/JavaScriptCore/
%{_libdir}/libjavascriptcoregtk-4.0.so
%{_libdir}/pkgconfig/javascriptcoregtk-4.0.pc
%{_datadir}/gir-1.0/JavaScriptCore-4.0.gir

%files plugin-process-gtk2
%{_libexecdir}/webkit2gtk-4.0/WebKitPluginProcess2

%files doc
%dir %{_datadir}/gtk-doc
%dir %{_datadir}/gtk-doc/html
%{_datadir}/gtk-doc/html/webkit2gtk-4.0/
%{_datadir}/gtk-doc/html/webkitdomgtk-4.0/

%changelog
* Mon Oct 09 2017 Tomas Popela <tpopela@redhat.com> - 2.14.7-3
- Update the bundled brotli and woff2 to the latest releases due to
  woff2's license incompatibility with WebKitGTK+ project
- Resolves: rhbz#1500368

* Fri Jun 16 2017 Tomas Popela <tpopela@redhat.com> - 2.14.7-2
- Fix a CLoop patch that was not correctly backported from upstream, causing
  crashes on big endian machines
- Resolves: rhbz#1442160

* Thu Jun 01 2017 Tomas Popela <tpopela@redhat.com> - 2.14.7-1
- Update to 2.14.7
- Backport more of a11y fixes from upstream
- Fix JSC crashes on big endian arches
- Resolves: rhbz#1442160

* Wed May 10 2017 Milan Crha <mcrha@redhat.com> - 2.14.6-6
- Add upstream patch to fix login to Google account
- Resolves: rhbz#1448192

* Wed Apr 26 2017 Tomas Popela <tpopela@redhat.com> - 2.14.6-5
- Don't require icu libraries that are bundled
- Resolves: rhbz#1414413

* Tue Apr 25 2017 Tomas Popela <tpopela@redhat.com> - 2.14.6-4
- Use the right function for removing from provides
- Resolves: rhbz#1383614

* Mon Apr 24 2017 Tomas Popela <tpopela@redhat.com> - 2.14.6-3
- Bundle only needed icu libraries
- Don't list bundled icu libraries in provides
- Resolves: rhbz#1383614

* Mon Apr 24 2017 Tomas Popela <tpopela@redhat.com> - 2.14.6-2
- Bundle icu57
- Resolves: rhbz#1414413

* Mon Apr 10 2017 Tomas Popela <tpopela@redhat.com> - 2.14.6-1
- Update to 2.14.6
- Resolves: rhbz#1440681
- Don't crash is no render is available in AX render object
- Resolves: rhbz#1437672

* Tue Mar 21 2017 Tomas Popela <tpopela@redhat.com> - 2.14.5-5
- Add more Coverity scan fixes
- Remove icu from sources
- Resolves: rhbz#1383614

* Mon Mar 13 2017 Tomas Popela <tpopela@redhat.com> - 2.14.5-4
- Add some Coverity scan fixes
- Resolves: rhbz#1383614

* Tue Feb 28 2017 Tomas Popela <tpopela@redhat.com> - 2.14.5-3
- Add explicit requires of webkitgtk4-jsc for -devel and -plugin-process-gtk2
  subpackages (found by rpmdiff).
- Resolves: rhbz#1383614

* Mon Feb 20 2017 Tomas Popela <tpopela@redhat.com> - 2.14.5-2
- Remove bundled ICU and require libicu57
- Resolves: rhbz#1383614

* Thu Feb 16 2017 Kalev Lember <klember@redhat.com> - 2.14.5-1
- Update to 2.14.5
- Resolves: rhbz#1383614

* Fri Feb 10 2017 Tomas Popela <tpopela@redhat.com> - 2.14.4-1
- Initial RHEL packaging
- Temporary bundling icu57 until rhbz#1414413 is resolved
- Resolves: rhbz#1383614
