#!/bin/sh

set -xe

cd /tmp
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local \
      -DCMAKE_UNITY_BUILD=ON -DCMAKE_UNITY_BUILD_BATCH_SIZE=32 \
      -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror -D_GLIBCXX_ASSERTIONS" \
      -DSWIG_COMPILE_FLAGS="-O1 -Wno-unused-parameter" \
      -DSPHINX_FLAGS="-W -T -j4" \
      /io
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
