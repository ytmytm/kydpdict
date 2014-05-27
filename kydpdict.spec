Summary:	Frontend to various dictionaries
Summary(pl.UTF-8):	Interfejs do różnych słowników
Name:		kydpdict
Version:	0.9.5
Release:	1
License:	GPL
Group:		Applications/Dictionaries
URL:		http://members.elysium.pl/ytm/html/kydpdict.html
Prefix:		%{_prefix}
Source0:	http://members.elysium.pl/ytm/src/%{name}-%{version}.tar.bz2
Source1:	%{name}.desktop
Source2:	%{name}.png
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  qt-devel >= 3.3.2
BuildRequires:  qt-linguist >= 3.3.2

%description
Kydpdict is a Qt frontend for various dictionaries. It supports free SAP
dictionary, commercial Young Digital Poland Collins and Langenscheidt
Polish-English/German dictionaries and commercial PWN Oxford English/Polish
dictionaries released in 2003 and 2004.

%description -l pl.UTF-8
Kydpdict to oparty na Qt interface pozwalający korzystać z różnych słowników.
Obsługiwane są: darmowy słownik SAP, komercyjne słowniki Collins i Langenscheidt
wydane przez Young Digital Poland oraz słowniki angielsko-polskie PWN Oxford
wydane w 2003 i 2004 roku.

%prep
%setup -q

%build
%{__aclocal} --acdir=config
%{__autoconf}
%configure \
	--with-qt-libraries=/usr/lib \
	--with-qt-includes=/usr/include/qt

%{__make}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir} \
	$RPM_BUILD_ROOT{%{_pixmapsdir},%{_desktopdir},%{_datadir}/%{name}}

install src/kydpdict $RPM_BUILD_ROOT%{_bindir}

install %{SOURCE1} $RPM_BUILD_ROOT%{_desktopdir}
install %{SOURCE2} $RPM_BUILD_ROOT%{_pixmapsdir}
install src/kydpdict_pl.qm $RPM_BUILD_ROOT%{_datadir}/%{name}
install lib/dvp_* $RPM_BUILD_ROOT%{_datadir}/%{name}

install -d $RPM_BUILD_ROOT%{_mandir}{/man1,/pl/man1}
install contrib/kydpdict.1 $RPM_BUILD_ROOT%{_mandir}/man1
install contrib/kydpdict.pl.1 $RPM_BUILD_ROOT%{_mandir}/pl/man1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc {AUTHORS,COPYING,README,ChangeLog,INSTALL,TODO}
%attr(755,root,root) %{_bindir}/*
%{_datadir}/%{name}/*
%{_desktopdir}/*
%{_pixmapsdir}/*
%{_mandir}/man*/*
%lang(pl) %{_mandir}/pl/man?/*
