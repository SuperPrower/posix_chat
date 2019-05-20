Name:           posix_chat
Version:        0.1
Release:        1%{?dist}
Summary:        Simple chat application based on POSIX sockets

License:        GPLv3+
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  make
BuildRequires:	gcc

%description


%prep
%autosetup


%build
make %{?_smp_mflags}


%install
%make_install


%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}


%changelog
* Sun May 19 2019 user
- 
