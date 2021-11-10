#!/bin/sh

set -xe

cd /tmp
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror" /io
make install
make tests
ctest --output-on-failure --timeout 100 ${MAKEFLAGS}

uid=$1
gid=$2
if test -n "${uid}" -a -n "${gid}"
then
  sudo cp -r ~/.local/share/*/doc/html /io
  sudo chown -R ${uid}:${gid} /io/html
fi
