Name:           posix_chat
Version:        0.1
Release:        1%{?dist}
Summary:        

License:        
URL:            
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  make
BuildRequires:	gcc
Requires:       

%description


%prep
%autosetup


%build
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%make_install


%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}



%changelog
* Sun May 19 2019 user
- 
