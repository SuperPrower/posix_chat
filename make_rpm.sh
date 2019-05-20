#!/usr/bin/env sh

# Check that prerequisites are installed
for i in "rpmbuild" "rpmdev-setuptree"
do
	command -v $i >/dev/null 2>&1 || { echo >&2 "$i is required but not installed. Aborting."; exit 1; }
done

rpmdev-setuptree

make tarball || { echo >&2 "Unable to make source tarball"; exit 1; }

cp rpm/posix_chat.spec ~/rpmbuild/SPECS/
cp posix_chat-0.1.tar.gz ~/rpmbuild/SOURCES/

rpmbuild -bb ~/rpmbuild/SPECS/posix_chat.spec || { echo >&2 "Unable to build .rpm file"; exit 1; }

echo "Done! Install .rpm using 'sudo rpm -Uvh ~/rpmbuild/RPMS/XXX/posix_chat-0.1-YYYWHATEVER.rpm'"
